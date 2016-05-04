#include <mpi.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>



int main(int argc, char** argv) {
    int msg , ierr;
    int number = 100;  
    int rand_range = 100;
    int chunk_size = 10;
    int root_size;
    MPI_Status status;
    MPI_Request request;
    clock_t begin, end;
    double time_spent;
    

    // Initialize the MPI environment
    ierr = MPI_Init(NULL, NULL);

    // Get the number of processes
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    // printf("size :%d\n", size );
    // Get the rank of the process
    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);
    
    int send_size = number/size  ; 
    // send array
    int recv_array[send_size]; 
    int root_array[send_size + number%size];
    
    if(rank == 0 ){
        int i , r , j ; 
        srand(time(NULL));

        for (j = 1; j < size; ++j)
        {
            if(send_size <= chunk_size){
                int randarray[send_size];
                for (i = 0 ; i < send_size ;i++ ){
                    randarray[i]= rand()%100 + 1;
                    // printf("rand%d\n", r);
                }
                //send array size
                ierr=  MPI_Send(&send_size, 1, MPI_INT, j, 0, MPI_COMM_WORLD);
                //send actual array
                ierr=MPI_Send(randarray, send_size, MPI_INT, j, 1, MPI_COMM_WORLD);
            }
            else{
                int k ; 
                int itr = send_size / chunk_size;
                for (k = 0 ; k < itr ; k++){
                    int randarray[chunk_size];
                    for (i = 0 ; i < chunk_size ;i++ ){
                        randarray[i]= rand()%100 + 1;
                        // printf("rand%d\n", r);
                    }
                    //send array size
                    ierr=  MPI_Send(&chunk_size, 1, MPI_INT, j, 0, MPI_COMM_WORLD);
                    //send actual array
                    ierr=MPI_Send(randarray, chunk_size, MPI_INT, j, 1, MPI_COMM_WORLD);       
                }
                if(send_size % chunk_size >0){
                    int remain = send_size % chunk_size;
                    int randarray[remain];
                    for (i = 0 ; i < remain ;i++ ){
                        randarray[i]= rand()%100 + 1 ;
                        // printf("rand%d\n", r);
                    }
                    //send array size
                    ierr=  MPI_Send(&remain, 1, MPI_INT, j, 0, MPI_COMM_WORLD);
                    //send actual recv_array
                    ierr=MPI_Send(randarray, remain, MPI_INT, j, 1, MPI_COMM_WORLD);   
                }
            }
        }
        root_size = send_size + number%size;        
        if(root_size < chunk_size){
            int i ; 
            int randarray[root_size];
            for (i = 0 ; i < root_size ;i++ ){
                root_array[i]= rand()%100 + 1;
                // printf("rand%d\n", r);
            }
            // printf("!! proc %d's array is size %d and goes from %d to %d\n",rank, root_size, root_array[0], root_array[(root_size-1)]);
            fflush(stdout);
        }
        else{
            int k , i ; 
            int itr = root_size / chunk_size;
            for (k =0 ; k < itr ; k++){
                for (i = 0 ; i < chunk_size ;i++ ){
                        root_array[k*chunk_size + i]= rand()%100 + 1;
                        // printf("rand%d\n", r);
                }
                i = 101 ; 
            }
            if(root_size % chunk_size >0){
                for (i = 0 ; i < root_size % chunk_size ;i++ ){
                        root_array[itr*chunk_size + i]= rand()%100 + 1;
                        // printf("rand%d\n", r);
                }
            }
            // printf("!! proc %d's array is size %d and goes from %d to %d\n", rank,root_size, root_array[0], root_array[(root_size-1)]);
            fflush(stdout);
        } 

    } 
    else{
    // int p ;
    // for (p = 1 ; p < size ; p++){
    //     if(rank == p){
            int recv_pending = send_size;
            int recv_size;
            ierr=MPI_Recv(&recv_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            if(recv_size == send_size){    
                ierr=MPI_Recv(recv_array , recv_size, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
                // printf("!! proc %d's array is size %d and goes from %d to %d\n",rank, recv_size, recv_array[0], recv_array[(recv_size-1)]);
                fflush(stdout);
            }
            else{
                while(recv_pending > 0){
                    int temp[recv_size]; 
                    ierr=MPI_Recv(temp , recv_size, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
                    int itr; 
                    for(itr = 0 ; itr < recv_size;itr++){
                        recv_array[send_size - recv_pending + itr] = temp[itr]; 
                    }
                    recv_pending = recv_pending - recv_size;
                    if(recv_pending > 0){
                        ierr=MPI_Recv(&recv_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
                    }
                }
                // printf("!! proc %d's array is size %d and goes from %d to %d\n",rank, send_size, recv_array[0], recv_array[(send_size-1)]);
                fflush(stdout);
            }        
    //     }
    // }
    }       
    // distribution complete

    begin = clock();

    // simple internal sort -> 
    if(rank == 0){
        int i, j, count;
        int bucket[100];
        for(i=0;i<100;i++){
            bucket[i] = 0 ;
        }
        for(i=0;i<root_size;i++){
            bucket[root_array[i]-1]++ ;
        }
        count = 0;
        for(i = 0; i < 100 ; i++){
             for(j=0;j<bucket[i];j++){
                root_array[count+j] = i + 1; 
             }
             count += bucket[i];   
        }
        // printf("count%d\n", count);
        // printf("!! proc %d's array is size %d and goes from %d to %d\n",rank, send_size + number%size, root_array[0], root_array[(send_size + number%size-1)]);
        fflush(stdout);       
    }
    else{
        int i, j, count;
        int bucket[100];
        for(i=0;i<100;i++){
            bucket[i] = 0 ;
        }
        for(i=0;i<send_size;i++){
            bucket[recv_array[i]-1]++ ;
        }
        count = 0;
        for(i = 0; i < 100 ; i++){
             for(j=0;j<bucket[i];j++){
                recv_array[count+j] = i + 1; 
             }
             count += bucket[i];   
        }
        
        // printf("!! proc %d's array is size %d and goes from %d to %d\n",rank, send_size, recv_array[0], recv_array[(send_size-1)]);
        fflush(stdout);
    }
    ////////////////////////////////////

    // <<<<<<<<<<<<<<<<<<<<-----bitonic sort ->>>>>>>>>>>>
    int i, j, d, sz, k;
    d = 0 ;
    sz = size;
    while(sz != 1){
        sz = sz/2;
        d++;
    }
    // printf("d _<%d\n", d);
    for (k = 0 ; k < d ; k++){
        for (j = k ; j > -1 ; j-- ){

            int i_1 = ((rank & (1<<(k+1)))>0);
            int j_1 =  ((rank & (1<<j))> 0 );
            
            int r2 = rank^(1<<j) ;  
            int bucket[100];
            int bucket_rec[100];
            for(i=0;i<100;i++){
                bucket[i] = 0 ;

            }
            if(rank ==0 ){

                for(i=0;i<root_size;i++){
                    bucket[root_array[i]-1]++ ;
                }
                // printf("i = %d, j = %d, %d sending to %d\n", i, j, rank, r2);

                MPI_Send(bucket,100,MPI_INT,r2,2,MPI_COMM_WORLD);
                MPI_Irecv(bucket_rec,100,MPI_INT,r2,2,MPI_COMM_WORLD,&request);    
                MPI_Wait(&request , &status);
                for(i=0;i<100;i++){
                    bucket[i] = bucket[i] + bucket_rec[i];
                }
                // printf("ranks %d from %d\n", rank,r2);
                fflush(stdout);

                if(i_1 == j_1 ){
                // min 
                    int k = 0 ;
                    int l = 0 ;  
                    while(k < root_size){
                        if(bucket[l] > 0){
                            bucket[l]--;
                            root_array[k] = l+1 ; 
                            k++; 
                        }
                        else{
                           l++; 
                        }    
                    }     
                }  
                else{
                    // max
                    int k = 0 ;
                    int l = 99 ;  
                    while(k < root_size){
                        if(bucket[l] > 0){
                            bucket[l]--;
                            root_array[k] = l+1 ; 
                            k++; 
                        }
                        else{
                           l--; 
                        }    
                    }

                }
            }
            else{
                for(i=0;i<send_size;i++){
                    bucket[recv_array[i]-1]++ ;
                }
                // printf("%d sending to %d", rank, r2);
                MPI_Send(bucket,100,MPI_INT,r2,2,MPI_COMM_WORLD);
                MPI_Irecv(bucket_rec,100,MPI_INT,r2,2,MPI_COMM_WORLD,&request);    
                MPI_Wait(&request , &status);

                for(i=0;i<100;i++){
                    bucket[i] = bucket[i] + bucket_rec[i];
                }
                // printf("ranks %d from %d\n", rank,r2);
                fflush(stdout);

                if(i_1 == j_1 ){
                // min 
                    int k = 0 ;
                    int l = 0 ;  
                    while(k < send_size){
                        if(bucket[l] > 0){
                            bucket[l]--;
                            recv_array[k] = l+1 ; 
                            k++; 
                        }
                        else{
                           l++; 
                        }    
                    }     
                }  
                else{
                    // max
                    int k = 0 ;
                    int l = 99 ;  
                    while(k < send_size){
                        if(bucket[l] > 0){
                            bucket[l]--;
                            recv_array[k] = l+1 ; 
                            k++; 
                        }
                        else{
                           l--; 
                        }    
                    }

                }   
            }        
        }
    }
    
    if(rank == 0){
        int i, j, count;
        int bucket[100];
        for(i=0;i<100;i++){
            bucket[i] = 0 ;
        }
        for(i=0;i<root_size;i++){
            bucket[root_array[i]-1]++ ;
        }
        count = 0;
        for(i = 0; i < 100 ; i++){
             for(j=0;j<bucket[i];j++){
                root_array[count+j] = i + 1; 
             }
             count += bucket[i];   
        }
    }
    else{
        int i, j, count;
        int bucket[100];
        for(i=0;i<100;i++){
            bucket[i] = 0 ;
        }
        for(i=0;i<send_size;i++){
            bucket[recv_array[i]-1]++ ;
        }
        count = 0;
        for(i = 0; i < 100 ; i++){
             for(j=0;j<bucket[i];j++){
                recv_array[count+j] = i + 1; 
             }
             count += bucket[i];   
        }
    }    

    MPI_Barrier(MPI_COMM_WORLD);
    // printf("MPI_Barrier\n");
    MPI_Barrier(MPI_COMM_WORLD);
    end = clock();
    printf("time spent : %f\n", (double)(end - begin) / CLOCKS_PER_SEC );    
    sleep(1);

    // print final arrays -->>>>>>>>>>>>
    // for (k= 0 ; k < size; k++){
    //     if(rank == k ){   
    //         if(k==0){
    //             printf("rank %d\n", k);
    //             for (i = 0; i < root_size; ++i)
    //             {
    //                 /* code */
    //                 printf("%d-",root_array[i]);
    //             }
    //             printf("\n" );
    //             fflush(stdout);
    //         }
    //         else{
    //             printf("rank %d\n", k);
    //             for (i = 0; i < send_size; ++i)
    //             {
    //                 /* code */
    //                 printf("%d-",recv_array[i]);
    //             }
    //             printf("\n" );
    //             fflush(stdout);   
    //         }
    //     }
    //     MPI_Barrier(MPI_COMM_WORLD);

    // }    
    // Finalize the MPI environment.
    MPI_Finalize();
} 