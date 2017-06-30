#include <unordered_map>
#include <string>
#include "functions.h"
#include "timer.h"
#include "parser.h"
#include <netinet/tcp.h>
#include <arpa/inet.h>   //for inet_aton
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;

extern int errno ;
int id;
int flag;
int myId;
int timeOut;
int majority;
int votesRecv;
int listening_portno;
char ** ip_array ;
char ** ip_controller ;
char * sending_port;
int no_of_proxy;
char delimChar='s';
char * tokenCheckIp;
int soc[PEERS];
int arr[PEERS];
int votes[PEERS];
int flagTG[PEERS];
double hardness[2];
float incomingRate[PEERS];
float sum_incoming_rate;
time_t startTime;
FILE *fptr;

struct clientDetails{
    int sockfd;
    int ip1;
    int ip2;
    int ip3;
    int ip4;
    int port;
};
unordered_map<clientDetails *,int> ipToid;
int connectedClients; // for asigining array indices to ipToid
void* controller(void *);

void* start_logging( void* ) {
    debug_printf("starting logging\n");
    // init_logger();
    // debug_printf("starting logging 1\n");
    start_timer();
}

void readID(){
	debug_printf("In read from file\n");
    fptr = fopen("IdStore","r");
    if(fptr != NULL){
        char c = fgetc(fptr);
        id = atoi(&c);
        // fscanf(fptr,"%d",&id);
        debug_printf("Read ID value %d in file\n",id);
        fclose(fptr);
    }
    else{
        debug_printf("Read ID -- else -- errno is : %d # text is : %s\n",errno,strerror(errno));
    }
    // fclose(fptr);
}

void writeID(){
	// debug_printf("In write to file -- 1\n");
    fptr = fopen("IdStore","w");
	// debug_printf("In write to file -- 2\n");
    if(fptr != NULL){
		// debug_printf("In write to file -- 3\n");
        fprintf(fptr,"%d",id);
		// debug_printf("In write to file -- 4\n");
		// debug_printf("In write to file -- 5\n");
        debug_printf("Written ID value %d in file\n",id);
        fclose(fptr);
    }
    else{
		debug_printf("Write to file failed\n");	
    }
    // debug_printf("In write to file -- 7 -- else -- errno is : %d # text is : %s\n",errno,strerror(errno));
    // fclose(fptr);
}

void calcSoc(){
    // calculate share of capacity
    // debug_printf("In CalcSOC\n");
    // debug_printf("Calculating SOC\n");
    //for(int i=0; i<no_of_proxy; i++){
    //    soc[counter] = 100;
    //}

	if(id != myId){
		return;
	}
    
    sum_incoming_rate = 0;

    for(int i=0; i<no_of_proxy; i++){
        incomingRate[i] = 0;
        // debug_printf("##### incoming rate i = %d #####\n",i,incomingRate[i]);
    }

    for(int i=0; i<no_of_proxy; i++){
        incomingRate[i] = incomingR[i];
        // debug_printf("##### incoming rate i = %d #####\n",i,incomingRate[i]);
    }

    for(int i=0; i<no_of_proxy; i++){
        sum_incoming_rate+=incomingRate[i];
        // debug_printf("##### incoming rate i = %d #####\n",i,incomingRate[i]);
    }

    // debug_printf("##### Sum incoming rate = %d #####\n",sum_incoming_rate);

    if(sum_incoming_rate==0){
        for(int i=0; i<no_of_proxy; i++){
            soc[i]=round(1/no_of_proxy * capacity);
            // debug_printf("## in 1 ## SOC calculated for peer %d = %d\n",i,soc[i]);          
        }
    }

    else{
        for(int i=0; i<no_of_proxy; i++){
            if(incomingRate[i]==0){
                soc[i]=1;
            }
            else {
                soc[i]=round(incomingRate[i]/sum_incoming_rate * capacity);
            }
            // debug_printf("## in 2 ## SOC calculated for peer %d = %d\n",i,soc[i]);  
        }
    }
}

void* calculate_soc(void*) {
    //int val = *((int*) args);
    // debug_printf("Calculating SOC before sleep\n");
    sleep(1);
    // debug_printf("the value of d is %d\n",d);
    // debug_printf("Calculating SOC\n");
    // if(val>=0 && val<no_of_proxy){
    calcSoc();
    // }
    // pthread_exit(NULL);
}

void sendID(char * ip_array_n, char delim){

    // char delim='t';
    int sockfd, portnum, n;
    struct sockaddr_in server_addr;
    // sending_port is already filled by the parser
    portnum = atoi(sending_port);
    /* Create client socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        debug_printf( "ERROR opening socket while writing to backup\n");
        exit(1);
    }
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if(!inet_aton( ip_array_n  , &server_addr.sin_addr))
    {
        debug_printf( "ERROR invalid server IP address\n");
        debug_printf( ip_array_n);
        exit(1);
    }
    server_addr.sin_port = htons(portnum);
    // Connect once to the current ip_array_n and keep on sending
    // visitor_count array at the below infinite while loop
    // debug_printf("print statement before connect\n");
    // int ret = connect(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr));
    if(connect(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr)) >= 0)
    {
        // debug_printf("print statement after connect\n");
        // the time frequnecy value is important 
        // 0,5 seconds does not work where as 1s works 
        // should be same as frequency of clearing incoming
        float sec= 1;       // time frequency in which to communicate
        float sec_frac = 0.0;
        debug_printf( "connected %s \n" , ip_array_n);
        struct timespec tim, rem;
        tim.tv_sec = sec;
        tim.tv_nsec = sec_frac*1000000000;
            
        n = write(sockfd, &delim, sizeof(char));
        if (n < 0) { debug_printf("ERROR writing to peer socket\n"); }
        n = write(sockfd, &myId , sizeof(int));
        if (n < 0) { debug_printf("ERROR writing to peer socket\n"); }
        debug_printf("Sent %c to %s\n",delim,ip_array_n);
        
    }else{
        debug_printf( "ERROR connecting to a peer in write - no : %d, text : %s\n",errno,strerror(errno));
        /* sleep(1); */
        /* exit(1); */
    }
    sleep(1);
    debug_printf( "disconnected %s in write\n" , ip_array_n);
    close( sockfd );
}

int readFromClient( struct clientDetails * cd ) {
    /* This functions reads data from two types of clients 
     * that connect to TokenGen
     * Two types of clients are:
     *  1 Capacity Estimator
     *  2 other peer proxy */
    // make buffer to store the data from client

    // debug_printf("In read for Backup\n");

    int clientSocketFD = cd->sockfd;
    /* debug_printf( "yo yo thread id %d\n", clientSocketFD ); */
    int bytes = LIMIT * sizeof(int);
    int*  buffer = (int *) malloc( bytes );
    int bytesRead;
    int prev_bytesRead=0;
    bzero(buffer,bytes);
    while( ( bytesRead = read( clientSocketFD, buffer, sizeof(char) ) ) > 0)
    {
        /* debug_printf( "bytesRead %d \n", bytesRead ); */
        //the fist character indicates if the data is from
        //a CapacityEstimator for TokenGen
        //c - capacity data from CapacityEstimator
        //h - hardness data form CapacityEstimator
        //i - incoming rate data from peer TokenGen
        //in all above communicatin the data count will be less than 20
        //if >20  - its peer_v_count from TokenGen

        if( *(char*)buffer == 'c' ){ // if content of buffer == c
            bzero(buffer,bytes);
            if( ( bytesRead = read( clientSocketFD, buffer, sizeof(int)) ) > 0){
                capacity = *buffer;
            }
            debug_printf( "data from capacity estimator in c %d\n", capacity );
            capacity = 720;
        }

        else if( *(char*)buffer == 'h' ){
            bzero(buffer,bytes);
            if( ( bytesRead = read( clientSocketFD, buffer, sizeof(int)) ) > 0){
                hardness[0] = *buffer;
            }
            bzero(buffer,bytes);
            if( ( bytesRead = read( clientSocketFD, buffer, sizeof(int)) ) > 0){
                hardness[1] = *buffer;
            }
            debug_printf("data from capacity estimator in h\n");
            // error here due to not handling buffer which is not decimal
            // hardness[0] = stod( ((char*)buffer)+1 );
            // hardness[1] = stod( strchr( ((char*)buffer)+2 , ' ')  );
            hardness[0] = 1;
            hardness[1] = 1;
            debug_printf( "hardness %f %f \n", hardness[0], hardness[1] );
        }

        else if( *(char*)buffer == 'a' ){ // if content of buffer == c
            // debug_printf( "Heartbeat received from primary\n");
            if(id == myId){
                debug_printf( "Primary received A\n");
                if(flag == 1){
                    char delim = 't';
                    sendID(ip_controller[1-myId],delim);
                }
            }
            else{
                debug_printf( "Backup received A\n");
                change_time(&startTime);             
                if(timeOut == 1){
                	int temp = 1-myId;
                    set_value(&id,temp);
                    set_value(&timeOut,2);
                    debug_printf("Received A at Backup after Timeout\n");
                }   
            }
        }

        else if( *(char*)buffer == 'p' ){
            int tempId;
            bzero(buffer,bytes);
            if( ( bytesRead = read( clientSocketFD, buffer, sizeof(int)) ) > 0){
                tempId = *buffer;
            }
            if(id == myId){
                debug_printf( "Primary received P\n");
            }
            else{
                if(votes[tempId]==0){
                    change_values(&votesRecv,1);
                    set_value(&votes[tempId],1);      
                }
                debug_printf( "Received positive response from TG %d\n",tempId);
            }
        }

        else if( *(char*)buffer == 'n' ){
            int tempId;
            bzero(buffer,bytes);
            if( ( bytesRead = read( clientSocketFD, buffer, sizeof(int)) ) > 0){
                tempId = *buffer;
            }
            if(id == myId){
                debug_printf( "Primary received N\n");
            }
            else{
                debug_printf( "Received negative response from TG %d\n",tempId);
            }
        }
        
        else if( *(char*)buffer == 't' ){
            int localId;
            char delim = 'z';
            bzero(buffer,bytes);
            if( ( bytesRead = read( clientSocketFD, buffer, sizeof(int)) ) > 0){
                localId = *buffer;
            }
            if(id == myId) {
                //handle
                debug_printf( "Primary received Controller Change msg\n");
            }
            else {
                //handle
                debug_printf( "Controller Changes at Backup with newId = %d prevId = %d\n" , localId,id);
            }
            set_value(&id,localId);
            writeID();
            debug_printf( "New Controller id = %d\n" , id);
            sendID(ip_controller[1-myId],delim);
        }

        else if( *(char*)buffer == 'y' ){
            int tempId;
            bzero(buffer,bytes);
            if( ( bytesRead = read( clientSocketFD, buffer, sizeof(int)) ) > 0){
                tempId = *buffer;
            }
            if(id == myId){
                set_value(&flagTG[tempId],0);
                debug_printf( "TG Controller Change Ack from id : %d at Primary\n" , tempId);
            }
            else{
                debug_printf( "TG Controller Change Ack from id : %d at Backup\n" , tempId);
            }
        }

        else if( *(char*)buffer == 'z' ){
            int localId;
            bzero(buffer,bytes);
            if( ( bytesRead = read( clientSocketFD, buffer, sizeof(int)) ) > 0){
                localId = *buffer;
            }
            if(id == myId){
                set_value(&flag,0);
                debug_printf( "Controller Change Ack for id : %d at Primary\n" , localId);
            }
            else{
                debug_printf( "Controller Change Ack for id : %d at Backup\n" , localId);
            }
        }

        else if( *(char*)buffer == 'i' ){
            int tempId;
            bzero(buffer,bytes);
            if( ( bytesRead = read( clientSocketFD, buffer, sizeof(int)) ) > 0){
                // copy content of buffer to incoming_peers
                tempId = *buffer;
                /* debug_printf( "buf %d %d\n" , ipToid[cd] , incoming_peers[ ipToid[cd] ] ); */
            }
            bzero(buffer,bytes);
            if( ( bytesRead = read( clientSocketFD, buffer, sizeof(int)) ) > 0){
                // copy content of buffer to incomingRate
                incomingR[tempId] = *buffer;
                debug_printf( "buf -- controller received inc rate -- %d from TG id %d\n", incomingR[tempId], tempId);
                calcSoc();
            }
            if(id == myId) {
                //handle
            }
            else {
                //handle
                // it can send a changeId msg to TG to let it know that it is not the Primary Controller
            }
        }
        /*
        else if( bytesRead > 20 ) {
            // debug_printf( "read from %d.%d.%d.%d arrayIdx %d\n", cd->ip1, cd->ip2, cd->ip3, cd->ip4 , ipToid[cd] ); 
            int j;
            memcpy( visitor_count[ ipToid[cd] ]+(prev_bytesRead/4) , buffer , bytesRead );
            prev_bytesRead += bytesRead;
            prev_bytesRead = (prev_bytesRead >= 4000 ) ? 0:prev_bytesRead;
            if( prev_bytesRead == 0 ) {
                // debug_printf( "once cycle done \n"); 
                // debug print the array being read
                // for( j =0 ; j < LIMIT ; j++ ){ 
                //     if (peer_v_count[ ipToid[cd] ][j] != 0 ||  visitor_count[j]!= 0 ) 
                //         debug_printf( "(%d)%d,%d\n", j, peer_v_count[ ipToid[cd] ][j], visitor_count[j] ); 
                // } 
            }
            if(id == myId) {
                //handle
            }
            else {
                //handle
            }
        }
        */
        else{
            debug_printf( "bytesRead in else %d \n", bytesRead );
        }
        // Exit once the communication is over.
    }
    debug_printf( "gonna shutdown read thread \n");
    // 2- shutdown both send and recieve functions
    return close( clientSocketFD );
}

void * ThreadWorker( void * threadArgs) {
    // debug_printf("In Thread Worker\n");
    readFromClient( (struct clientDetails *) threadArgs);
    pthread_exit(NULL);
}

void* create_server_socket() {
    //
    // Creates server socket for communication between Proxy1 and Proxy2, and CapacityEstimator
    //

    // int flag = *((int*) args);

    // debug_printf("In create server socket\n");

    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    /* First call to socket() function */
    // debug_printf("going to open a socket\n");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // debug_printf("socket opened\n");

    if (sockfd < 0) {
        // clear
        perror("ERROR opening socket");
        exit(1);
    }
    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    /* listening_portno = 5007; */


    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    // serv_addr.sin_addr.s_addr = inet_addr("10.129.28.87");
    // debug_printf("filled up server structure\n");

    // char *primaryC = inet_ntoa(serv_addr.sin_addr);
    // debug_printf("primary controller ip address is : %s\n",primaryC);
    // debug_printf("primary controller ip address is : %lu\n",serv_addr.sin_addr.s_addr);
    // 

    serv_addr.sin_port = htons(listening_portno);
    // debug_printf("controller's ip is %lu\n",serv_addr.sin_addr.s_addr);
    // debug_printf("controller's port number is %d\n",listening_portno);

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    /* Now start listening for the clients, here process will
     * go in sleep mode and will wait for the incoming connection
     */
    listen(sockfd, 20);
    clilen = sizeof(cli_addr);
    int option = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*) &option, sizeof(option)) < 0) {
        printf("setsockopt failed\n");
        close(sockfd);
        exit(2);
    }
    //setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &option, sizeof(int));
    /* Accept actual connection from the client */
    pthread_t threadId;
    while (1) { 
        // debug_printf("checkpoint 1\n");
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        // debug_printf("checkpoint 2\n");
        if (newsockfd < 0) {
            // perror("ERROR on accept");
            debug_printf("ERROR on accept\n");

            exit(1);
        }
        // print the ip
        debug_printf("peer connected: %d.%d.%d.%d\n",
                 (int)(cli_addr.sin_addr.s_addr&0xFF),
                 (int)((cli_addr.sin_addr.s_addr&0xFF00)>>8),
                     (int)((cli_addr.sin_addr.s_addr&0xFF0000)>>16),
                     (int)((cli_addr.sin_addr.s_addr&0xFF000000)>>24));
        struct clientDetails * cd;
        cd = (clientDetails * ) malloc( sizeof( struct clientDetails ) );
        cd->sockfd = newsockfd;
        cd->ip1 = (int)(cli_addr.sin_addr.s_addr&0xFF);
        cd->ip2 = (int)((cli_addr.sin_addr.s_addr&0xFF00)>>8);
        cd->ip3 = (int)((cli_addr.sin_addr.s_addr&0xFF0000)>>16);
        cd->ip4 = (int)((cli_addr.sin_addr.s_addr&0xFF000000)>>24);
        // add cd to a hash
        ipToid[ cd ] = connectedClients;
        connectedClients++;
        if(pthread_create(&threadId, NULL, ThreadWorker, (void *)cd) < 0)
        {
            debug_printf("Thread creation failed");
            exit(1);
        }

    }
}

void writeToServer(int counter){
    // FUNCTION DESC:
    // write visitor array to peer proxy 
    // ( which is a server as far as 'this' proxy is concerned, therefore the 
    // function name )
    // This function runs as a separate thread.for each peer
    // i.e.: There will be as many instance of this function as the
    // number of peer TokenGen
    // debug_printf("In write to server counter is %d peer is %s\n",counter,ip_array[counter]);

    int sockfd, portnum, n;
    struct sockaddr_in server_addr;
    // sending_port is already filled by the parser
    portnum = atoi(sending_port);
    /* Create client socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        debug_printf( "ERROR opening socket while writing to server\n");
        exit(1);
    }
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if(!inet_aton( ip_array[counter], &server_addr.sin_addr))
    {
        debug_printf( "ERROR invalid server IP address\n");
        debug_printf( ip_array[counter]);
        exit(1);
    }
    server_addr.sin_port = htons(portnum);
    // Connect once to the current ip_array[counter] and keep on sending
    // visitor_count array at the below infinite while loop
    // debug_printf( "Going to connect ip %s \n" , ip_array[counter]);
    debug_printf("going to connect \n");
    if(connect(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr)) >= 0)
    {
        // the time frequnecy value is important 
        // 0,5 seconds does not work where as 1s works 
        // should be same as frequency of clearing incoming
        float sec= 1;       // time frequency in which to communicate
        float sec_frac = 0.0;
        debug_printf( "connected %s \n" , ip_array[counter]);
        struct timespec tim, rem;
        tim.tv_sec = sec;
        tim.tv_nsec = sec_frac*1000000000;
        while( 1)
        {
            /* debug_printf( "writing to %s \n" , ip_array[counter]); */
            // debug print the visitor_count being written
            /* for(int j =0 ; j < LIMIT ; j++ ){ */
            /*     if ( visitor_count[j]!= 0 ) */
            /*         debug_printf( "(%d) vc %d\n", j, visitor_count[j] ); */
            /* } */
            // sent incoming with 'i' indicator

            // -- needs to write only share --
        
            // write share  
            // int nb=0;

            if(id != myId){
				return;
			}

            n = write(sockfd, &delimChar, sizeof(char));
            if (n < 0) { debug_printf("ERROR writing to peer socket\n"); }
            // nb+=n;
            n = write(sockfd, &myId , sizeof(int) );
            if (n < 0) { debug_printf("ERROR writing to peer socket\n"); }
            n = write(sockfd, &soc[counter] , sizeof(int) );
            if (n < 0) { debug_printf("ERROR writing to peer socket\n"); }
            debug_printf("written id %d, delim %c, soc val %d, to peer %d - %s\n",myId,delimChar,soc[counter],counter,ip_array[counter]);
            // nb+=n;
            // debug_printf("writing nb = %d bytes\n",nb);
            // sent visitor queue
            //n = write(sockfd, visitor_count[counter] , 1000 * sizeof(int) );
            //if (n < 0) { debug_printf("ERROR writing to peer socket\n"); }
            // connect as a client;
            nanosleep( &tim, &rem );
        }
    }else{
        debug_printf( "ERROR connecting to a peer %s\n",ip_array[counter]);
        /* sleep(1); */
        /* exit(1); */
    }
    debug_printf( "disconnected %s \n" , ip_array[counter]);
    close( sockfd );
}

void* queue_sender(void* args) {/*{{{*/
    int val = *((int*) args);
    // debug_printf("In QSender before sleep %d\n",val);
    
    sleep(1);
    // debug_printf("In QSender after sleep\n");
    
    writeToServer(val);
    
}/*}}}*/

void changeController(char *ip_array_n){
    // FUNCTION DESC:
    // write visitor array to peer proxy 
    // ( which is a server as far as 'this' proxy is concerned, therefore the 
    // function name )
    // This function runs as a separate thread.for each peer
    // i.e.: There will be as many instance of this function as the
    // number of peer TokenGen
    
    debug_printf("Sending changeController request to %s\n",ip_array_n);
    
    char delim='x';
    int sockfd, portnum, n;
    struct sockaddr_in server_addr;
    // sending_port is already filled by the parser
    portnum = atoi(sending_port);
    /* Create client socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        debug_printf( "ERROR opening socket while sending changeController request\n");
        exit(1);
    }
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if(!inet_aton( ip_array_n  , &server_addr.sin_addr))
    {
        debug_printf( "ERROR invalid server IP address\n");
        debug_printf( ip_array_n);
        exit(1);
    }
    server_addr.sin_port = htons(portnum);
    // Connect once to the current ip_array_n and keep on sending
    // visitor_count array at the below infinite while loop
    if(connect(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr)) >= 0)
    {
        // the time frequnecy value is important 
        // 0,5 seconds does not work where as 1s works 
        // should be same as frequency of clearing incoming
        float sec= 1;       // time frequency in which to communicate
        float sec_frac = 0.0;
        debug_printf( "connected %s \n" , ip_array_n);
        struct timespec tim, rem;
        tim.tv_sec = sec;
        tim.tv_nsec = sec_frac*1000000000;

        //---------------------why is while there---------------------------//

        n = write(sockfd, &delim, sizeof(char));
        if (n < 0) { debug_printf("ERROR writing to peer socket\n"); }
        n = write(sockfd, &myId , sizeof(int));
        if (n < 0) { debug_printf("ERROR writing to peer socket\n"); }
        debug_printf("Change controller request sent\n");
        //wait for response from TG

    }else{
        debug_printf( "ERROR connecting to a peer in write\n");
        /* sleep(1); */
        /* exit(1); */
    }
    sleep(1);
    debug_printf( "disconnected %s in write\n" , ip_array_n);
    close( sockfd );
}

void* x_sender( void * args) {/*{{{*/
    debug_printf("In XSender\n");
    changeController((char *) args);
    pthread_exit(NULL);
}/*}}}*/

void printValues(){
    debug_printf("\n\nPrinting values from config file\n");
    for(int i=0;i<no_of_proxy;i++){
        debug_printf("Peer %s\n",ip_array[i]);
    }
    for(int i=0;i<2;i++){
        debug_printf("Controller %s\n",ip_controller[i]);
    }
    debug_printf("My Id %d\n",myId);
    debug_printf("Sending Port %s\n",sending_port);
    debug_printf("Token Check IP %s\n",tokenCheckIp);
    debug_printf("No of Proxy %d\n",no_of_proxy);
    debug_printf("Majority %d\n",majority);
    debug_printf("Listening Port No %d\n",listening_portno);
    debug_printf("Done printing values from config file\n\n");
}

void sendToBackup(char *ip_array_n){
   
    // debug_printf("In Write To Backup peer is %s\n",ip_array_n);
    
    char delim='a';
    int sockfd, portnum, n;
    struct sockaddr_in server_addr;
    // sending_port is already filled by the parser
    portnum = atoi(sending_port);
    /* Create client socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        debug_printf( "ERROR opening socket while writing to backup\n");
        exit(1);
    }
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if(!inet_aton( ip_array_n  , &server_addr.sin_addr))
    {
        debug_printf( "ERROR invalid server IP address\n");
        debug_printf( ip_array_n);
        exit(1);
    }
    server_addr.sin_port = htons(portnum);
    // Connect once to the current ip_array_n and keep on sending
    // visitor_count array at the below infinite while loop
    // debug_printf("print statement before connect\n");
    // int ret = connect(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr));
    
	if(connect(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr)) >= 0)
	{
		// debug_printf("print statement after connect\n");

	    // the time frequnecy value is important 
	    // 0,5 seconds does not work where as 1s works 
	    // should be same as frequency of clearing incoming
	    float sec= 1;       // time frequency in which to communicate
	    float sec_frac = 0.0;
	    debug_printf( "connected %s \n" , ip_array_n);
	    struct timespec tim, rem;
	    tim.tv_sec = sec;
	    tim.tv_nsec = sec_frac*1000000000;
	    while(1){

	    	if(id != myId){
				return;
			}

	      	n = write(sockfd, &delim, sizeof(char));
	       	if (n < 0) { 
                debug_printf("ERROR writing to peer socket\n");
                break; 
            }
	       	debug_printf("Sending A to %s\n",ip_array_n);
	       	// n = write(sockfd, &myId , sizeof(int));
	       	// if (n < 0) { debug_printf("ERROR writing to peer socket\n"); }
               
            nanosleep( &tim, &rem );
            
		}
	            
	}else{
	        debug_printf( "ERROR connecting to a peer in write - no : %d, text : %s\n",errno,strerror(errno));
	        /* sleep(1); */
	    	/* exit(1); */
	}
    
    debug_printf( "disconnected %s in write\n" , ip_array_n);
    close( sockfd );
}

void send_heartbeat( void * args ) {/*{{{*/
    while(1){
    	sleep(1);
        sendToBackup( (char *) args );
        if(id != myId){
            return;
        }        
    }
}/*}}}*/

void switch_controller(){

	debug_printf("In switch controller code\n");
    char delim = 't';
	
	for (int counter = 0; counter < no_of_proxy ; counter++)
    {
        set_value(&flagTG[counter],1);
        sendID(ip_array[counter],delim);
        debug_printf("TG %d flag value changed\n",counter);
    }
    set_value(&flag,1);
    sendID(ip_controller[1-myId],delim);
    while(1){
        sleep(1);
        int c = 0;
        for (int counter = 0; counter < no_of_proxy ; counter++)
        {
            if(flagTG[counter] == 1){
                c++;
                sendID(ip_array[counter],delim);
            }
        }
        if(c == 0){
        	debug_printf("#### Controller ID Changed at all TGs to %d ####\n",myId);
            break;
        }
    }
}

void timeout(){
	// can initiate a thread to wait for heartbeat msg from previous Primary Controller
    debug_printf("Timeout reached\n");
    set_value(&timeOut,1);
    set_value(&id,-1);
    set_value(&votesRecv,0);
    int trigger = 3;
    
    pthread_t send_x[PEERS];
    
    for (int counter = 0; counter < no_of_proxy ; counter++)
    {
        set_value(&votes[counter],0);
    }

    for (int counter = 0; counter < no_of_proxy ; counter++)
    {
        debug_printf("For loop: counter is : %d\n",counter);
        pthread_create(&send_x[counter], NULL, x_sender, (void *)ip_array[counter]);
    }
    
    debug_printf("Awaiting response from TGs\n");
    change_time(&startTime);
    
    while(time(NULL)-startTime < trigger) {
    	// if it receives append entries req within this duration, it shud become backup again
        if(timeOut == 2){
            debug_printf("Received A at Backup\n");
            set_value(&timeOut,0);
            return;
        }
        // debug_printf("In timer, waiting for votes from TGs\n");
    }
    debug_printf("!! votesRecv = %d, majority = %d !!\n",votesRecv,majority);
    if(votesRecv >= majority){
    	set_value(&id,myId);
        writeID();
    	debug_printf("!! Votes Recv exceeds majority, Controller id = %d !!\n",id);
    	switch_controller();
    }
    else{
    	debug_printf("!! Votes Recv not reached majority -- redo !!\n");
    	// redo same process
    }
}

void* timer_fn(){
    // debug_printf("timer_fn checkpoint 1\n");
    int trigger = 4;
    change_time(&startTime);

    while(time(NULL)-startTime < trigger) {
        
    }
    timeout();
    debug_printf("# Leaving timer_fn #\n");
}

void* controller(void*){
	while(1){
        debug_printf("## Id = %d, My Id = %d ##\n",id,myId); 
		if(id == myId){
	        debug_printf("In Controller primary code\n");
	        pthread_t calc_soc;
	        pthread_create(&calc_soc, NULL, calculate_soc, (void*) NULL);
	        pthread_t send_queue[PEERS];
	        for (int counter = 0; counter < no_of_proxy ; counter++)
	        {
	             arr[counter] = counter; 
	             pthread_create(&send_queue[counter], NULL, queue_sender, &(arr[counter]));
	        }
	        send_heartbeat((void*)ip_controller[(1-myId)]);
            debug_printf("Leaving Controller Primary code\n");\
	    }
	    else{
	        debug_printf("In Controller backup code\n");
            change_time(&startTime);
	        timer_fn();
            debug_printf("Leaving Controller Backup code\n");\
	    }
	}
}

int main(void) {
    flag = 0;
    timeOut = 0;
    numbytes = 0;
    incoming = 0;
    failing = 0;
    total_in = 0;
    total_out = 0;
    capacity = 720;
    hardness[0] = 1;
    hardness[1] = 1;
    sum_incoming_rate = 0;
    connectedClients = 0;
    current_time = 0;
    int counter;

    // myId to be read from config file

    for (counter = 0; counter < PEERS; counter++)
    {
        incomingRate[counter]=0;
        soc[counter]=1;
        flagTG[counter]=0;
        votes[counter]=0;
    }

    for (counter = 0; counter < LIMIT; counter++)
    {
        // visitor_count[counter] = 0;  // Initialize visitor_count
        int i ;
        for( i = 0 ; i < PEERS; i++ )
            visitor_count[i][counter] = 0; // replace with memset
    }

    // parse the proxy.conf file to get the values 
    parse_config_file();
    init_logger();
    readID();
    majority = (no_of_proxy/2)+1;
    printValues();

    pthread_t timer_log;
    pthread_create( &timer_log, NULL, start_logging, (void*) NULL);
    // pthread_join(timer_log, NULL);

    pthread_t cont_th;
    pthread_create( &cont_th, NULL, controller, (void*) NULL);

    create_server_socket();
}