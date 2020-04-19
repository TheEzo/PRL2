/*
 * Viditelnost
 * Author: Willaschek Tomas (xwilla00)
 */

#include <mpi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>

#define TAG 0
#define NONUM -1
//#define TESTING = 1

using namespace std;

int main(int argc, char **argv)
{
    int numprocs, myid, mynumber, first_val, iterations;
    double angle, max_angle;
    vector<int> my_numbers;

    MPI_Status stat;
    // inicializace MPI a ridicich hodnot
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    // nacteni dat ze souboru a distribuce mezi procesory
    if(myid == 0){
        int rest = 0, proc_cnt, number_count;
        vector<int> numbers;
        ifstream stream;
        stringstream ss;
        string item;
        char delimiter = ',';
        stream.open("numbers", ios::in);

        stream >> item;
        ss.str(item);
        while(getline(ss, item, delimiter))
            numbers.push_back(stoi(item));
        number_count = numbers.size() - 1;
        // vypocet, kolik kazdy procesor musi prijmout prvku
        proc_cnt = number_count % numprocs ? number_count / numprocs + 1 : number_count / numprocs;

        // poslani nulteho prvku vsem procesorum vsem nulty prvek na spocitani uhlu
        first_val = numbers[0];
        for(int i = 0; i < numprocs; i++)
            MPI_Send(&first_val, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);

        // poslani poctu prvku, ktere procesory prijmou musi prijmout
        for(int i = 0; i < numprocs; i++)
            MPI_Send(&proc_cnt, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);

        // posilani cisel jednotlivym procesorum, preskoceni prvniho cisla, to neni treba
        rest = numprocs * proc_cnt;
        int index;
        for(int i = 0; i < numprocs; i++){
            for(int j = 0; j < proc_cnt && rest; j++){
                index = proc_cnt * i + j + 1;
                if(index < numbers.size())
                    mynumber = numbers[index];
                else
                    break;
                MPI_Send(&mynumber, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);
                rest--;
            }
        }
        // zbytek vyplnime vatou
        for(int j = numprocs-1; j >= 0; j--)
            for(int i = 0; i < proc_cnt && rest; i++){
                mynumber = NONUM;
                MPI_Send(&mynumber, 1, MPI_INT, j, TAG, MPI_COMM_WORLD);
                rest--;
            }
    }

    double start;
    if(myid == 0)
        start = MPI_Wtime();

    // nastaveni pocatecniho bodu
    MPI_Recv(&first_val, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat);
    // nastaveni postu iteraci pro prijem prvku
    MPI_Recv(&iterations, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat);
    // prijeti vsech potrebnych cisel
    for(int i = 0; i < iterations; i++){
        MPI_Recv(&mynumber, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat);
        my_numbers.push_back(mynumber);
    }

    // vypocet uhlu cisel prirazenych procesoru
    vector<double> my_angles;
    int num_index;
    for(int i = 0; i < iterations; i++){
        num_index = iterations * myid + i + 1; // index na vstupu
        if(my_numbers[i] == NONUM)
            my_angles.push_back(NONUM);
        else
            my_angles.push_back(atan((my_numbers[i] - first_val) / (double)num_index));
        max_angle = i ? max(my_angles[i], my_angles[i-1]) : my_angles[i];
    }

    vector<double> node_angles = {max_angle};
    int size;
    // UpSweep
    for(int d = 2; d <= numprocs; d = d*2){
        for(int i = numprocs; i > 0; i -= d){
            if(myid == i-d/2-1){
                MPI_Send(&max_angle, 1, MPI_DOUBLE, i-1, TAG, MPI_COMM_WORLD);
            }
            if(myid == i-1){
                MPI_Recv(&angle, 1, MPI_DOUBLE, i-d/2-1, TAG, MPI_COMM_WORLD, &stat);
                max_angle = max(node_angles[node_angles.size() - 1], angle);
                node_angles.push_back(angle);
            }
        }
    }

    // Nahrazeni posledni prvku za neutralni
    if(myid == numprocs - 1){
        node_angles.push_back(-5);
    }
    // DownSweep
    for(int d = numprocs; d >= 2; d = d/2){
        for(int i = numprocs; i > 0; i -= d){
            if(myid == i-d/2-1){
                MPI_Recv(&angle, 1, MPI_DOUBLE, i-1, TAG, MPI_COMM_WORLD, &stat);
                node_angles.push_back(angle);
            }
            if(myid == i-1){
                size = node_angles.size() - 1;
                angle = node_angles[size];
                MPI_Send(&angle, 1, MPI_DOUBLE, i-d/2-1, TAG, MPI_COMM_WORLD);
                node_angles.pop_back();
                max_angle = max(angle, node_angles[size-1]);
                node_angles.pop_back();
                node_angles.push_back(max_angle);
            }
        }
    }
    // sekvencni porovnani prvku jendoho procesoru
    max_angle = node_angles[1];
    int res;
    for(int i = 0; i < iterations; i++){
        if(my_numbers[i] == NONUM){
            res = NONUM;
        }
        else{
            if(my_angles[i] > max_angle){
                res = 1;
                max_angle = my_angles[i];
            }
            else
                res = 0;
        }
        MPI_Send(&res, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD);
    }

    double end;
    if(myid == 0)
        end = MPI_Wtime();

    #ifdef TESTING
    if(myid == 0){
        ofstream f;
        f.open("test.out", ios::app);
        f << ";" << end - start;
        f.close();
    }
    #endif

    if(myid == 0){
        cout << "_";
        for(int i = 0; i < numprocs; i++){
            for(int j = 0; j < iterations; j++){
                MPI_Recv(&res, 1, MPI_INT, i, TAG, MPI_COMM_WORLD, &stat);
                if(res != NONUM)
                    cout << (res ? ",v" : ",u");
            }
        }
        cout << endl;
    }

    MPI_Finalize();
    return 0;
}
