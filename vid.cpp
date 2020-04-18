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

using namespace std;

int main(int argc, char *argv[])
{
    int numprocs, myid, mynumber, first_val, iterations;
    double angle;

    MPI_Status stat;
    // inicializace MPI a ridicich hodnot
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

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

        // posilani cisel jednotlivym procesorum, preskoceni prvniho, to neni treba
        for(int i = 0; i < number_count; i++){
            rest = i % numprocs;
            mynumber = numbers[i+1];
            MPI_Send(&mynumber, 1, MPI_INT, rest, TAG, MPI_COMM_WORLD);
        }
        // zbytek vyplnime -1
        for(int i = rest+1; i < numprocs; i++){
            mynumber = NONUM;
            MPI_Send(&mynumber, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);
        }
    }

    // nastaveni pocatecniho bodu
    MPI_Recv(&first_val, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat);
    // nastaveni postu iteraci pro prijem prvku
    MPI_Recv(&iterations, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat);
    // prijeti vsech potrebnych cisel
    vector<int> my_numbers;
    for(int i = 0; i < iterations; i++){
        MPI_Recv(&mynumber, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat);
        my_numbers.push_back(mynumber);
    }

    // vypocet uhlu
    vector<double> my_angles;
    int num_index;
    for(int i = 0; i < iterations; i++){
        num_index = i * numprocs + myid + 1; // index na vstupu
        if(my_numbers[i] == NONUM)
            my_angles.push_back(NONUM);
        else
            my_angles.push_back(atan((my_numbers[i] - first_val) / (double)num_index));
    }

    // ziskani vsech uhlu a sdileni sveho
    double all_angles[iterations * numprocs + 1];
    for(int i = 0; i < iterations; i++){
        // vsichni poslou vsem
        for(int j = 0; j < numprocs; j++){
            angle = my_angles[i];
            MPI_Send(&angle, 1, MPI_DOUBLE, j, TAG, MPI_COMM_WORLD);
            MPI_Recv(&angle, 1, MPI_DOUBLE, j, TAG, MPI_COMM_WORLD, &stat);
            num_index = i * numprocs; // index v poli vsech uhlu
            all_angles[num_index + j] = angle;
        }
    }

    // porovnani a zjisteni viditelnosti, odeslani vysledku
    int state; // 1 = 'v', 0 = 'u', -1 = pro vyplneni poctu procesoru
    for(int i = 0; i < iterations; i++){
        bool visible = true;
        num_index = i * numprocs + myid; // index v poli vsech uhlu
        if(all_angles[num_index] == NONUM)
            state = -1;
        else {
            // max_prev_angle
            for (int j = num_index - 1; j >= 0 && visible; j--)
                if (all_angles[j] >= all_angles[num_index])
                    visible = false;
            state = visible ? 1 : 0;
        }
        MPI_Send(&state, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD);
    }

    // sber a vypis dat
    if(myid == 0){
        for(int i = 0; i < iterations; i++)
            for(int j = 0; j < numprocs; j++)
                cout << all_angles[i * numprocs + j] << endl;
        cout << "_";
        for(int i = 0; i < iterations; i++){
            for(int j = 0; j < numprocs; j++){
                MPI_Recv(&state, 1, MPI_INT, j, TAG, MPI_COMM_WORLD, &stat);
                if(state == 1)
                    cout << ",v";
                else if(state == 0)
                    cout << ",u";
            }
        }
        cout << endl;
    }

    MPI_Finalize();
    return 0;
}
