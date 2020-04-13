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

#define TAG 0
#define NONUM -1

using namespace std;

int main(int argc, char *argv[])
{
    int numprocs, myid, neighnumber, mynumber, number_count;
    numprocs = 4; // TODO
    int proc_cnt, first_val, index = 0, iterations, rest = 0;

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




//    // nacteni vstupniho souroru procesem "0" a rozeslani cisel jednotlivym procesum, vcetne "0"
//    // vypis cisel ze souboru na stdout
//    if(myid == 0){
//        int number, receiver = 0;
//        ifstream stream;
//        stream.open("numbers", ios::in);
//        while(stream.good()){
//            number = stream.get();
//            #ifndef TESTING
//            if(receiver)
//                cout << " ";
//            #endif
//            if(!stream.good())
//                break;
//            #ifndef TESTING
//            cout << number;
//            #endif
//            MPI_Send(&number, 1, MPI_INT, receiver++, TAG, MPI_COMM_WORLD);
//        }
//        #ifndef TESTING
//        cout << endl;
//        #endif
//        stream.close();
//    }
//
//    MPI_Recv(&mynumber, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat);
//
//    int odd_max = 2 * (numprocs / 2) - 1;
//    int even_max = 2 * ((numprocs - 1) / 2);
//
//    bool printed = false;
//    double start;
//    if(myid == 0)
//        start = MPI_Wtime();
//    // zacatek razeni, maximalne -- pocet_cisel / 2
//    for(int k = 0; k < numprocs/2; k++){
//
//        // sude odeslou lichym a cekaji na vysledek
//        // ve skutecnosti indexujeme procesory od "0", takze se vlastne jedna o liche procesory pri indexaci od "1"
//        if(myid < odd_max && !(myid % 2)){
//            MPI_Send(&mynumber, 1, MPI_INT, myid + 1, TAG, MPI_COMM_WORLD);
//            MPI_Recv(&mynumber, 1, MPI_INT, myid + 1, TAG, MPI_COMM_WORLD, &stat);
//        }
//        else if(myid <= odd_max){ // liche porovnaji a vrati mensi
//            MPI_Recv(&neighnumber, 1, MPI_INT, myid - 1, TAG, MPI_COMM_WORLD, &stat);
//            if(mynumber < neighnumber){
//                MPI_Send(&mynumber, 1, MPI_INT, myid - 1, TAG, MPI_COMM_WORLD);
//                mynumber = neighnumber;
//            }
//            else
//                MPI_Send(&neighnumber, 1, MPI_INT, myid - 1, TAG, MPI_COMM_WORLD);
//        }
//
//        // liche odeslou sudym a cekaji na vysledek
//        if(myid < even_max && myid % 2){
//            MPI_Send(&mynumber, 1, MPI_INT, myid + 1, TAG, MPI_COMM_WORLD);
//            MPI_Recv(&mynumber, 1, MPI_INT, myid + 1, TAG, MPI_COMM_WORLD, &stat);
//        }
//        else if(myid <= even_max && myid != 0){ // sude porovnaji a vrati mensi, 0 nic, protoze se posila na +1
//            MPI_Recv(&neighnumber, 1, MPI_INT, myid - 1, TAG, MPI_COMM_WORLD, &stat);
//            if(mynumber < neighnumber){
//                MPI_Send(&mynumber, 1, MPI_INT, myid - 1, TAG, MPI_COMM_WORLD);
//                mynumber = neighnumber;
//            }
//            else
//                MPI_Send(&neighnumber, 1, MPI_INT, myid - 1, TAG, MPI_COMM_WORLD);
//        }
//    }
//
//
//
//    int *final = new int [numprocs];
//    for(int i = 1; i < numprocs; i++){
//        if(myid == i)
//            MPI_Send(&mynumber, 1, MPI_INT, 0, TAG,  MPI_COMM_WORLD);
//        if(myid == 0){
//            MPI_Recv(&neighnumber, 1, MPI_INT, i, TAG, MPI_COMM_WORLD, &stat);
//            final[i] = neighnumber;
//        }
//    }
//    if(myid == 0){
//        final[0] = mynumber;
//        for(int i = 1; i < numprocs; i++){
//            cout << final[i] << endl;
//        }
//    }
//
    MPI_Finalize();
    return 0;
}
