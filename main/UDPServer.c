#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <esp_log.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "UDPServer.h"
#include "ErrorHandling.h"
#include "InOut.h"
#include "Globals.h"

static char tag[]="udpserver";
int mysocket;

char Versione[]={"Version 01.5\0"};


// UDP Listener
esp_err_t udp_server(void* args)
{
    struct sockaddr_in si_other;
    
    unsigned int slen = sizeof(si_other),recv_len;
    char buf[BUFLEN];
    
    //2018/01/27 Copiato da programma Visentin variabili per gestione messaggi in ingresso UDP
    //uint16_t l;
    
    // bind to socket
    ESP_LOGI(tag, "initializing: bind_udp_server port:%d", PORT_NUMBER);
    mysocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (mysocket < 0) {
        show_socket_error_reason(mysocket);
        return ESP_FAIL;
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_NUMBER);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(mysocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        show_socket_error_reason(mysocket);
        close(mysocket);
        return ESP_FAIL;
    } else {
        ESP_LOGI(tag,"initializing: socket created without errors");
        
        while(1)
        {
            ESP_LOGI(tag,"Waiting for incoming data...");
            printf("**************************************************\n\n");
            memset(buf,0,BUFLEN);
            
            if ((recv_len = recvfrom(mysocket, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
            {
                ESP_LOGE(tag,"recvfrom");
                break;
            }
            
            printf("\n**************************************************\n");
            ESP_LOGI(tag,"Received a packet from %s:%d",
                    inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
            ESP_LOGI(tag,"Data: %s -- %d" , buf, recv_len);
            
            //2018/01/27 Copiato da programma Visentin algoritmo per gestione messaggi in ingresso UDP
            // Set the NULL byte to avoid garbage in the read buffer
            if ((recv_len + 1) < BUFLEN)
                buf[recv_len + 1] = '\0';
            
            uint16_t cmd=*((uint16_t *) buf);
            //2018/01/27 test hashcode per invio stringhe
            //uint32_t hash_cmd=0;
            //for(uint16_t i=0; i<BUFLEN-1; i++){
            //    hash_cmd+= (uint16_t) buf[i];
            //}
            struct sockaddr_in ra;
            memset(&ra, 0, sizeof(struct sockaddr_in));
            ra.sin_family = AF_INET;
            ra.sin_addr = si_other.sin_addr;
            ra.sin_port = si_other.sin_port;
            //if(hash_cmd==49){
            //    ESP_LOGI(tag,"\"1\"\n");
            //    sendto(mysocket,Versione,sizeof(Versione),0,(struct sockaddr *) &ra,sizeof(ra));
            //}
            printf("comando inviato -> hashcode decimale ->> %i",cmd);
            switch(cmd)
            {
            	case 1:			// VERSIONE
                    ESP_LOGI(tag,"richiesta: versione\n");
                    sendto(mysocket,Versione,sizeof(Versione),0,(struct sockaddr *) &ra,sizeof(ra));
            		sendto(mysocket,(char *)&INOUTData,sizeof(INOUTData),0,(struct sockaddr *) &ra,sizeof(ra));
                    break;
                case 2:
                    ESP_LOGI(tag,"richiesta: set & update inout\n");
                    ESP_LOGI(tag,"recv_len=%i",recv_len);
                    if(recv_len>=6){ //aggiorno solo se ho ricevuto dei dati validi
                        recINOUTData.Outpins=buf[2]+(buf[3]<<8);
                        recINOUTData.PwmState=buf[4]+(buf[5]<<8);
                        SetINOUTData(NULL);
                    }
                    UpdateINOUTData(true);
                    sendto(mysocket,(char *)&INOUTData,sizeof(INOUTData),0,(struct sockaddr *) &ra,sizeof(ra));
                    break;
                case 3:
                    ESP_LOGI(tag,"richiesta: update inout\n");
                    ESP_LOGI(tag,"recv_len=%i",recv_len);
                    UpdateINOUTData(true);
                    sendto(mysocket,(char *)&INOUTData,sizeof(INOUTData),0,(struct sockaddr *) &ra,sizeof(ra));
                    break;
            	case 0x4100:  //NOTHING
                    ESP_LOGI(tag,"41 00\n");
            		//l=MessagePack(&DebugArray, DATA_DEBUG_STRUCT_PACK,MsgPackArray);
            		//sendto(mysocket,(char *)MsgPackArray,l,0,(struct sockaddr *) &ra,sizeof(ra));
            		break;
                case 0x0109:  // LETTURA VALORI FISICI
                    ESP_LOGI(tag,"01 09\n");
                    //l=sizeof(PhiValue);
                    //sendto(mysocket,(char *) PhiValue,l,0,(struct sockaddr *) &ra,sizeof(ra));
                    break;
                case 0x011a:  // LETTURA POTENZA
                    ESP_LOGI(tag,"01 1a\n");
                    //l=MessagePack((uint8_t *) PhyPower,POWER_PACK,MsgPackArray);
            		//sendto(mysocket,(char *)MsgPackArray,l,0,(struct sockaddr *) &ra,sizeof(ra));
                    break;
                case 0x011c:  // READ ENERGIA CALCOLATA
                    ESP_LOGI(tag,"01 1c\n");
                    //l=MessagePack((uint8_t *) PhyEnergy,ENERGY_PACK,MsgPackArray);
            		//sendto(mysocket,(char *)MsgPackArray,l,0,(struct sockaddr *) &ra,sizeof(ra));
                    break;
                case 0x6943: //test STRINGA 
                    ESP_LOGI(tag,"9F BF\n");
                    char risposta[] = "Ciao. Il server e' operativo";
                    //uint32_t res=*((uint32_t *) "A te!\n");
                    //sendto(mysocket,(char *)&res,sizeof(res),0,(struct sockaddr *) &ra,sizeof(ra));
            	    sendto(mysocket,(char *)&risposta,sizeof(risposta),0,(struct sockaddr *) &ra,sizeof(ra));
                    break;
                case 0x6542: //test STRINGA
                    ESP_LOGI(tag,"9F BF\n");
                    ESP_LOGI(tag,"%s",(char *)&INOUTData);
                    //char risposta2[] = "Tutte le mie funzionalita' sono operative (o almeno spero)";
                    //sendto(mysocket,(char *)&res,sizeof(res),0,(struct sockaddr *) &ra,sizeof(ra));
            	    sendto(mysocket,(char *)&INOUTData,sizeof(INOUTData),0,(struct sockaddr *) &ra,sizeof(ra));
                    break;
                default:
                    ESP_LOGI(tag,"altro=male\n");
                    sendto(mysocket,(char *) &cmd,sizeof(cmd),0,(struct sockaddr *) &ra,sizeof(ra));
            		break;
            }
        }
        
        close_socket(mysocket);
        return ESP_FAIL;
    }
    return ESP_OK;
}
