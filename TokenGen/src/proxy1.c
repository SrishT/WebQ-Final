#include <unordered_map>
#include <string>
#include <stdio.h>
#include "functions.h"
#include "timer.h"
#include "parser.h"
#include <errno.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>   //for inet_aton
using namespace std;

int id;
int myId;
int flag;
int prevId;
int localId;
int listening_portno;
char ** ip_array ;
char ** ip_controller ;
char * sending_port;
int no_of_proxy;
char delimChar='i';
char * tokenCheckIp;
// char * controllerIp;
float peer_incomingRate[PEERS];
float sum_peer_incoming_rate;
double hardness[2];
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

void* start_logging( void* ) {
    // debug_printf("Tokengen starting logging\n");
    start_timer();
}

void readID(){
    fptr = fopen("IdStore","r");
    if(fptr != NULL){
    	// debug_printf("In read id -- if\n");
        char c = fgetc(fptr);
        id = atoi(&c);
        // fscanf(fptr,"%d",&id);
    	fclose(fptr); 
    }
    else{
        // debug_printf("Read ID -- else -- errno is : %d # text is : %s\n",errno,strerror(errno));
    }
    // fclose(fptr);
}

void writeID(){
    fptr = fopen("IdStore","w");
    if(fptr != NULL){
    	// debug_printf("In write id -- if\n");
        fprintf(fptr,"%d",id);
    	fclose(fptr); 
    }
    else{
    	// debug_printf("In write id -- else -- errno is : %d # text is : %s\n",errno,strerror(errno));
    }
    // fclose(fptr);
}

void sendResponse(char* ip_array_n, char response){
    // FUNCTION DESC:
    // write visitor array to peer proxy 
    // ( which is a server as far as 'this' proxy is concerned, therefore the 
    // function name )
    // This function runs as a separate thread.for each peer
    // i.e.: There will be as many instance of this function as the
    // number of peer TokenGen

    // debug_printf("sendResponse : Sending reply to Controller %c\n",response);

    int sockfd, portnum, n;
    struct sockaddr_in server_addr;
    // sending_port is already filled by the parser
    portnum = atoi(sending_port);
    /* Create client socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        // debug_printf( "sendResponse : ERROR opening socket while writing to server\n");
        exit(1);
    }
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if(!inet_aton( ip_array_n  , &server_addr.sin_addr))
    {
        // debug_printf( "sendResponse :ERROR invalid server IP address\n");
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
        // debug_printf( "sendResponse :connected %s \n" , ip_array_n);
        struct timespec tim, rem;
        tim.tv_sec = sec;
        tim.tv_nsec = sec_frac*1000000000;
        // while(1)
        // {
            /* debug_printf( "writing to %s \n" , ip_array_n); */
            // debug print the visitor_count being written
            /* for(int j =0 ; j < LIMIT ; j++ ){ */
            /*     if ( visitor_count[j]!= 0 ) */
            /*         debug_printf( "(%d) vc %d\n", j, visitor_count[j] ); */
            /* } */
            // sent incoming with 'i' indicator
            n = write(sockfd, &response, sizeof(char));
            if (n < 0) { debug_printf("sendResponse :ERROR writing to peer socket\n"); }
            n = write(sockfd, &myId , sizeof(int) );
            if (n < 0) { debug_printf("sendResponse :ERROR writing to peer socket\n"); }
            // n = write(sockfd, &incoming , sizeof(int) );
            // if (n < 0) { debug_printf("ERROR writing to peer socket\n"); }
            // debug_printf("#NumBytes for inc %d\n",n);
            // numbytes+=n;
            // sent visitor queue
            // n = write(sockfd, visitor_count , 1000 * sizeof(int) );
            // if (n < 0) { debug_printf("ERROR writing to peer socket\n"); }
            // numbytes+=n;
            // connect as a client;
            // nanosleep( &tim, &rem );
        // }
    }else{
        // debug_printf( "sendResponse :ERROR connecting to a peer in write\n");
        /* sleep(1); */
        /* exit(1); */
    }
    sleep(1);
    // debug_printf( "sendResponse :disconnected %s in write\n" , ip_array_n);
    close( sockfd );
}

void writeToServer(void*){
    // FUNCTION DESC:
    // write visitor array to peer proxy 
    // ( which is a server as far as 'this' proxy is concerned, therefore the 
    // function name )
    // This function runs as a separate thread.for each peer
    // i.e.: There will be as many instance of this function as the
    // number of peer TokenGen

    // debug_printf("writeToServer : In writeToServer function\n");
    char *ip_array_n;
    int count = 0;
    // while(1){
    	count++;
    	// debug_printf("writeToServer : this is iteration numner %d\n",count);
        if(id != -1){
            ip_array_n = ip_controller[id];
        }
        else{
            ip_array_n = ip_controller[prevId];        	
        }

        // debug_printf("writeToServer :In write to server peer is %s\n",ip_array_n);
        // debug_printf("writeToServer :### writing should be done to controller = %s  with id = %d ###\n",ip_controller[id],id);
        int sockfd, portnum, n;
        struct sockaddr_in server_addr;
        // sending_port is already filled by the parser
        portnum = atoi(sending_port);
        /* Create client socket */
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
        {
            // debug_printf( "writeToServer :ERROR opening socket while writing to server\n");
            exit(1);
        }
        bzero((char *) &server_addr, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        if(!inet_aton( ip_array_n  , &server_addr.sin_addr))
        {
            // debug_printf( "writeToServer :ERROR invalid server IP address\n");
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
            float sec= 1; // time frequency in which to communicate
            float sec_frac = 0.0;
            // debug_printf( "writeToServer :connected %s \n" , ip_array_n);
            struct timespec tim, rem;
            tim.tv_sec = sec;
            tim.tv_nsec = sec_frac*1000000000;
            while(1)
            {
                /* debug_printf( "writing to %s \n" , ip_array_n); */
                // debug print the visitor_count being written
                /* for(int j =0 ; j < LIMIT ; j++ ){ */
                /*     if ( visitor_count[j]!= 0 ) */
                /*         debug_printf( "(%d) vc %d\n", j, visitor_count[j] ); */
                /* } */
                // sent incoming with 'i' indicator
                n = write(sockfd, &delimChar, sizeof(char));
                if (n < 0) {
                    // debug_printf("writeToServer :ERROR writing to peer socket\n");
                    if(flag == 1){
                    	set_value(&flag,0);
						break;                    	
                    }
                }
                n = write(sockfd, &myId , sizeof(int) );
                if (n < 0) { 
                    // debug_printf("ERROR writing to peer socket\n"); 
                    if(flag == 1){
                        set_value(&flag,0);
                        break;                      
                    }
                }
                n = write(sockfd, &incoming , sizeof(int) );
                if (n < 0) { 
                    // debug_printf("writeToServer :ERROR writing to peer socket\n"); 
                    if(flag == 1){
                    	set_value(&flag,0);
						break;                    	
                    } 
                }
                // debug_printf("writeToServer : Wrote myId : %d, inc rate : %d from id : %d\n",myId,incoming,id);
                numbytes+=n;
                // sent visitor queue
                /*
                n = write(sockfd, visitor_count , 1000 * sizeof(int) );
                if (n < 0) {
                    debug_printf("writeToServer :ERROR writing to peer socket\n");
                    if(flag == 1){
                    	set_value(&flag,0);
						break;                    	
                    } 
                }
                numbytes+=n;
                */
                // connect as a client;
                nanosleep( &tim, &rem );
            }
            // debug_printf("writeToServer : Outside inner while loop \n");
        }else{
            // debug_printf( "writeToServer :ERROR connecting to a peer in write\n");
            /* sleep(1); */
            /* exit(1); */
        }
        // debug_printf( "writeToServer :disconnected %s in write\n" , ip_array_n);
        close( sockfd );
    // }
}

void* queue_sender( void * args) {/*{{{*/
    // debug_printf("Tokengen : In QSender before sleep\n");
    sleep(1);
    // debug_printf("Tokengen : In QSender after sleep\n");
    writeToServer( args  );
}/*}}}*/
    
int readFromClient( struct clientDetails * cd ) {
    /* This functions reads data from two types of clients 
     * that connect to TokenGen
     * Two types of clients are:
     *  1 Capacity Estimator
     *  2 other peer proxy */
    // make buffer to store the data from client

    // debug_printf("readFromClient : In read from client\n");

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
            // debug_printf( "readFromClient :data from capacity estimator in c %d\n", capacity );
            capacity = 720;
        }
        else if( *(char*)buffer == 's' ){ // if content of buffer == s
            int tempId,tempSh;
            bzero(buffer,bytes);
            if( ( bytesRead = read( clientSocketFD, buffer, sizeof(int)) ) > 0){
                // copy content of buffer to incoming_peers
                tempId = *buffer;
                /* debug_printf( "buf %d %d\n" , ipToid[cd] , incoming_peers[ ipToid[cd] ] ); */
            }
            bzero(buffer,bytes);
            if( ( bytesRead = read( clientSocketFD, buffer, sizeof(int)) ) > 0){
                // copy content of buffer to incoming_peers
                tempSh = *buffer;
                /* debug_printf( "buf %d %d\n" , ipToid[cd] , incoming_peers[ ipToid[cd] ] ); */
            }
            // debug_printf( "readFromClient : data from controller in s %d # id %d --\n", tempSh,tempId);
            if((id == -1 && tempId == prevId ) || tempId == id){
                if(tempSh != 0){
                    share = tempSh;
                }
                // debug_printf( "readFromClient : SOC = %d from Controller Id = %d\n", share,tempId );
                change_time(&startTime);
                if(id == -1){
                    set_value(&id,prevId);
                    //come out of timeout state
                }
            }
            else if(id == -1 && tempId != prevId){
            	// tell controller to step down -- to be handled
            }
        }
        else if( *(char*)buffer == 'h' ){
        	bzero(buffer,bytes);
            // debug_printf("readFromClient : data from capacity estimator in h\n");
            if( ( bytesRead = read( clientSocketFD, buffer, sizeof(int)) ) > 0){
                hardness[0] = *buffer;
            }
            bzero(buffer,bytes);
            if( ( bytesRead = read( clientSocketFD, buffer, sizeof(int)) ) > 0){
                hardness[1] = *buffer;
            }
            // error here due to not handling buffer which is not decimal
            // hardness[0] = stod( ((char*)buffer)+1 );
            // hardness[1] = stod( strchr( ((char*)buffer)+2 , ' ')  );
            hardness[0] = 1;
            hardness[1] = 1;
            // debug_printf( "readFromClient : hardness %f %f \n", hardness[0], hardness[1] );
        }
        else if( *(char*)buffer == 'i' ){
            bzero(buffer,bytes);
            if( ( bytesRead = read( clientSocketFD, buffer, sizeof(int)) ) > 0){
                // copy content of buffer to incoming_peers
                incoming_peers[ipToid[cd]] = *buffer;
                // debug_printf( "readFromClient :buf %d %d\n" , ipToid[cd] , incoming_peers[ ipToid[cd] ] );
            }
        }
        else if( *(char*)buffer == 'x' ){
            char rep;
            bzero(buffer,bytes);
            if( ( bytesRead = read( clientSocketFD, buffer, sizeof(int)) ) > 0){
                // copy content of buffer to incoming_peers
                localId = *buffer;
                // debug_printf( "readFromClient : ## Received change controller request from controller id = %d ##\n" ,localId );
                if(id == -1){
                    rep = 'p';
                }
                else{
                    rep = 'n';
                }
                sendResponse(ip_controller[localId],rep);
            }
        }
        else if( *(char*)buffer == 't' ){ // if content of buffer == s
        	int temp;
            int rep = 'y';
            bzero(buffer,bytes);
            if( ( bytesRead = read( clientSocketFD, buffer, sizeof(int)) ) > 0){
                // copy content of buffer to incoming_peers
                temp = *buffer;
                set_value(&id,temp);
                writeID();
                change_time(&startTime);
                /* debug_printf( "buf %d %d\n" , ipToid[cd] , incoming_peers[ ipToid[cd] ] ); */
            }
            // debug_printf( "readFromClient : !! Controller Changed !!\n");
            // debug_printf( "readFromClient : New Controller ID = %d\n", id);
            set_value(&flag,1);
            sendResponse(ip_controller[id],rep);
            pthread_t send_queue;
    		pthread_create( &send_queue , NULL , queue_sender, (void *) NULL );
        }
        /*
        else if( bytesRead > 20 ) {
            // debug_printf( "read from %d.%d.%d.%d arrayIdx %d\n", cd->ip1, cd->ip2, cd->ip3, cd->ip4 , ipToid[cd] ); 
            int j;
            memcpy( peer_v_count[ ipToid[cd] ]+(prev_bytesRead/4) , buffer , bytesRead );
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
        }
        */
        else{
            // debug_printf( "readFromClient : bytesRead in else %d \n", bytesRead );
        }
        // Exit once the communication is over.
    }
    // debug_printf( "readFromClient:gonna shutdown read thread \n");
    // 2- shutdown both send and recieve functions
    return close( clientSocketFD );
}

void * ThreadWorker( void * threadArgs) {
    // debug_printf("ThreadWorker :In Thread Worker\n");
    readFromClient( (struct clientDetails *) threadArgs);
    pthread_exit(NULL);
}

void* create_server_socket(void*) {
    //
    // Creates server socket for communication between Proxy1 and Proxy2, and CapacityEstimator
    //

    // debug_printf("create_server_socket : In create server socket\n");

    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        // perror("create_server_socket :ERROR opening socket in create server socket\n");
        exit(1);
    }
    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    /* listening_portno = 5007; */


    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(listening_portno);

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        // perror("create_server_socket :ERROR on binding in create server socket\n");
        exit(1);
    }

    /* Now start listening for the clients, here process will
     * go in sleep mode and will wait for the incoming connection
     */
    listen(sockfd, 20);
    clilen = sizeof(cli_addr);
    int option = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*) &option,
            sizeof(option)) < 0) {
        // printf("create_server_socket :setsockopt failed in create server socket\n");
        close(sockfd);
        exit(2);
    }
    //setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &option, sizeof(int));
    /* Accept actual connection from the client */
    pthread_t threadId;
    while (1) {
        // debug_printf("create_server_socket : going to call accept()\n");
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        // debug_printf("create_server_socket : After accept()\n");

        if (newsockfd < 0) {
            // perror("create_server_socket:ERROR on accept in create server socket\n");
            exit(1);
        }
        // print the ip
        // debug_printf("create_server_socket : in read peer connected: %d.%d.%d.%d\n",
                 // (int)(cli_addr.sin_addr.s_addr&0xFF),
                 // (int)((cli_addr.sin_addr.s_addr&0xFF00)>>8),
                     // (int)((cli_addr.sin_addr.s_addr&0xFF0000)>>16),
                     // (int)((cli_addr.sin_addr.s_addr&0xFF000000)>>24));
        struct clientDetails * cd;
        cd = (clientDetails * ) malloc( sizeof( struct clientDetails ) );
        cd->sockfd = newsockfd;
        cd->ip1 = (int)(cli_addr.sin_addr.s_addr&0xFF);
        cd->ip2 = (int)((cli_addr.sin_addr.s_addr&0xFF00)>>8);
        cd->ip3 = (int)((cli_addr.sin_addr.s_addr&0xFF0000)>>16);
        cd->ip4 = (int)((cli_addr.sin_addr.s_addr&0xFF000000)>>24);
        // add cd to a hash
        
        // TBU by srishti -------------------------------------------
        ipToid[ cd ] = connectedClients;
        //-----------------------------------------------------------
        

        connectedClients++;
        if(pthread_create(&threadId, NULL, ThreadWorker, (void *)cd) < 0)
        {
            // debug_printf("create_server_socket :Thread creation failed");
            exit(1);
        }

    }
}

void timeout(){
    // debug_printf("timeout : Timeout reached in TokenGen\n");
    set_value(&prevId,id);
    set_value(&id,-1);

    // TG shud send a msg to backup
}

void* timer_fn(void*){
    // debug_printf("timer_fn checkpoint 1\n");
    int trigger = 3;
    while(1){
        //long long int counter_timerFn=1;
        // startTime = time(NULL);
        change_time(&startTime);

        while(time(NULL)-startTime < trigger) {
            // debug_printf("no Timeout\n");
            // debug_printf("the value of counter : %lld\n",counter_timerFn);
            // counter_timerFn++;
        }
        timeout();
    }
}

void printValues(){
    debug_printf("\n\nprintValues :Printing values from config file\n");
    debug_printf("printValues :Sending Port %s\n",sending_port);
    debug_printf("printValues :Token Check IP %s\n",tokenCheckIp);
    debug_printf("printValues :Primary Controller IP %d\n",ip_controller[id]);
    debug_printf("printValues :Backup Controller IP %d\n",ip_controller[1-id]);
    debug_printf("printValues :Listening Port No %d\n",listening_portno);
    debug_printf("printValues :Done printing values from config file\n\n");
}

int main(void) {/*{{{*/
    flag = 0;
    numbytes = 0;
    incoming = 0;
    failing = 0;
    total_in = 0;
    total_out = 0;
    capacity = 720;
    hardness[0] = 1;
    hardness[1] = 1;
    connectedClients = 0;
    current_time = 0;
    int counter;

    // Initialize
    // visitor_count - number of incoming at current TokenGen
    // peer_v_count - number of incoming at peers
    for (counter = 0; counter < LIMIT; counter++)
    {
        visitor_count[counter] = 0;  // Initialize visitor_count
        int i ;
        for( i = 0 ; i < PEERS; i++ )
        peer_v_count[i][counter] = 0; // replace with memset
    }

    // parse the proxy.conf file to get the values 

    parse_config_file();
    init_logger();
    readID();
    // printValues();

    pthread_t timer_log;
    pthread_create( &timer_log, NULL, start_logging, (void*) NULL);
    pthread_t make_connection;
    pthread_create(&make_connection, NULL, create_server_socket, (void*) NULL);
    pthread_t send_queue;
    pthread_create( &send_queue , NULL , queue_sender, (void *) NULL );
    pthread_t timer;
    pthread_create(&timer, NULL, timer_fn, (void*) NULL);

    while (FCGI_Accept() >= 0) {
        change_values(&incoming, 1);
        total_in++;

        // Updated on 17.04
        time_t currtime = time(NULL);

        // we need to find share of capacity (share)
        // hostIncomingRate is updated in timer.h
        // sum_peer_incoming_rate - is updated below
        int iter = 0;
        int j;
        for( j=0; j<PEERS; j++)
        {
            peer_incomingRate[j] = 0; // TODO use memset
        }
        // sum the times .. actual avg found later outside the loop
        for( j=0; j<PEERS; j++)
        {
            peer_incomingRate[j] = incoming_peers[j];
            /* debug_printf( "%d %d %f\n", j, incoming_peers[j] , peer_incomingRate[j]); */
        }

        if ( share == 0 ) {
            // share can never be 0
            share = 1;
        }

        for (iter = 0; iter < LIMIT; iter++) {
            // find the current used capacity for THIS "iter" time instant 
            //peerUsedCapacity = 0;

            int usedCapacity = 0;
            usedCapacity = get_array(&visitor_count[(current_time + iter) % LIMIT]);

            //for( j=0; j<PEERS; j++)
            //{
                //peerUsedCapacity += get_array( &peer_v_count[j][(current_time + iter) % LIMIT] );
            //}
            /* debug_printf( "uc-%d puc-%d share-%d iter-%d \n", usedCapacity, peerUsedCapacity, share , iter); */
            
            int total_usable_capacity = (share  - usedCapacity) ; // use a buffer here to compensate n/w delay!!!
            
            //if( peerUsedCapacity > 0 ){
                //excess_used = (capacity - share)-peerUsedCapacity;
                //total_usable_capacity += excess_used < 0 ? excess_used : 0;
            //}

            char* req_url = getenv("QUERY_STRING");
            int url;
            if(strcmp(req_url,"req1.php")==0)
                url=0;
            else
                url=1;
            if ( total_usable_capacity > 0 )
            {
                debug_printf( "time %d %d \n" , current_time , iter) ;
                // found the time at which the request is to be
                // sheduled , now update the visitor_count array
                update_array(&visitor_count[(current_time + iter) % LIMIT], hardness[url]); // increment by hardness
                break;
            }
        }
        if (iter == LIMIT) {
            printf("Content-Type:text/html\n\n");

            printf(
                    "<title>Frontend</title> <body>Server Totally Busy</body></html>");
        } else {
            float float_wait_time = iter;

            int time_to_wait = float_wait_time;

            //changes made to enable logging of avg wait time (following line is uncommented)
            change_float_values(&total_waiting_time, float_wait_time, 0);
            char* gt = get_token(time_to_wait);
            char* env_var = getenv("QUERY_STRING");
            char* request_limit = strchr(env_var, '=') + 1;
            char url_to_visit[100];
            if( strchr(env_var, 'M') > 0 || strchr(env_var, 'S') > 0 ){
                strcpy(url_to_visit, "http://");
                strcat( url_to_visit,  tokenCheckIp);
                strcat( url_to_visit, "/");
            }
            else{
                /* fired url http://<ip>/proxy1?limit=moodleS/moodle/ */
                /* http://<ip>/moodleS/moodle/ */
                strcpy(url_to_visit, "http://");
                strcat( url_to_visit,  tokenCheckIp);
                strcat( url_to_visit, "/test.php?limit=");
            }
            strcat(url_to_visit, (const char*) request_limit);
            /* debug_printf("%d %d\n", time_to_wait, (currtime+iter)%LIMIT ) ; */
            printf("Refresh: %d; url=%s&hash=%s&token=%s\n", time_to_wait,
                    url_to_visit,/*"aaa"*/getHash((unsigned char*) gt),
                    encrypt(gt));
            printf("JMeter: %0.3f; url=%s&hash=%s&token=%s\n", float_wait_time,
                    url_to_visit,/*"aaa"*/getHash((unsigned char*) gt),
                    encrypt(gt));
            printf("Content-Type:text/html\n\n");

            printf(
                    "<title>Frontend</title>"
                    "Total incoming this second: <b>%d</b> \n<br />"
                    "Total requests served this second: <b>%d</b> \n<br /> "
                    "Total incoming requests till now: <b>%d</b> \n<br /> "
                    "Total requests served till now: <b>%d</b> \n<br /> "
                    "currtime: <b>%ld</b> \n<br /> "
                    "You will be redirected in %d seconds<br/>"
                    "query limit : %s"
                    "</div></body>",
                    incoming,
                    outgoing,
                    total_in,
                    total_out,
                    currtime,
                    time_to_wait,
                    request_limit);
            printf(
                    "<br>"
                    "<a href='%s&hash=%s&token=%s\n' style='text-decoration:none;'>"
                    "<button style='cursor:pointer'>Click here to go without waiting</button>"
                    "</a></center></body></html>",
                    url_to_visit,
                    /*"aaa"*/getHash((unsigned char*) gt),
                    encrypt(gt));

            free(gt);
        }
    }

}/*}}}*/