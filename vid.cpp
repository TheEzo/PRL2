/*
 * Viditelnost
 * Author: Willaschek Tomas (xwilla00)
 *
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
    int numprocs, myid, neighnumber, mynumber, number_count;
    numprocs = 4; // TODO
    int proc_cnt, first_val, index = 0, iterations, rest = 0;
    double angle, tmp_angle, max_prev_angle = 0;

    MPI_Status stat;
    // inicializace MPI a ridicich hodnot
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    if(myid == 0){
        ifstream stream;
        stringstream ss;
        string item;
        char delimiter = ',';
        stream.open("numbers", ios::in);
        vector<int> numbers;
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
        for(int i = ++rest; i < numprocs; i++){
            mynumber = NONUM;
            MPI_Send(&mynumber, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);
        }
    }

    // nastaveni pocatecniho bodu
    MPI_Recv(&first_val, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat);
    // nastaveni postu iteraci pro prijem prvku
    MPI_Recv(&iterations, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat);
    // prijeti vsech potrebnych cisel
    vector<int> proc_numbers;
    for(int i = 0; i < iterations; i++){
        MPI_Recv(&mynumber, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat);
        proc_numbers.push_back(mynumber);
    }

    for(int i = 0; i < iterations; i++){
        angle = atan((proc_numbers[index] - first_val) / (myid )); // TODO x / index_prvku
        MPI_Send(&angle, 1, MPI_DOUBLE, 0, TAG, MPI_COMM_WORLD);

        if(myid == 0){
            for(int j = 0; j < numprocs; j++){
                MPI_Recv(&tmp_angle, 1, MPI_DOUBLE, j, TAG, MPI_COMM_WORLD, &stat);
                if(tmp_angle > max_prev_angle){
                    max_prev_angle = tmp_angle;
                    cout << "v,";
                }
                else
                    cout << "u,";
            }

        }
        index++;
    }

    MPI_Finalize();
    return 0;
}
