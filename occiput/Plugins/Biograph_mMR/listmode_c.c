
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <memory.h>


#define STATUS_SUCCESS              0
#define STATUS_IO_ERROR             1
#define STATUS_DECODE_ERROR         2 
#define STATUS_INITIALISATION_ERROR 3
#define STATUS_PARAMETER_ERROR      4
#define STATUS_UNHANDLED_ERROR      5

#define MAX_TIME_BINS       65536 
#define MAX_GATING_EVENTS   10000000

#define MAX_FRAMES_CYCLIC   4096 

/*
Return the status flags, so that they can be used by the (Python) wrapper to interpret 
the result of the function calls. 
*/
extern int status_success(int *status) 
{
    *status = STATUS_SUCCESS; 
    return STATUS_SUCCESS; 
}
extern int status_io_error(int *status)
{
    *status = STATUS_IO_ERROR; 
    return STATUS_SUCCESS; 
}
extern int status_decode_error(int *status)
{
    *status = STATUS_DECODE_ERROR; 
    return STATUS_SUCCESS; 
}
extern int status_initialisation_error(int *status)
{
    *status = STATUS_INITIALISATION_ERROR; 
    return STATUS_SUCCESS; 
}
extern int status_parameter_error(int *status)
{
    *status = STATUS_PARAMETER_ERROR; 
    return STATUS_SUCCESS; 
}
extern int status_unhandled_error(int *status)
{
    *status = STATUS_UNHANDLED_ERROR; 
    return STATUS_SUCCESS; 
}


/*
Return the integer parameter. This function is meant to test 
the (Python) wrapper of the library. 
*/
extern int echo(int *in, int *out)
{
    *out = *in; 
    return STATUS_SUCCESS; 
}


// BUFFER_SIZE_BATCH defines how many packets are read from the input stream and processed at once. 
#define BUFFER_SIZE_BATCH      1  // 100000 (bigger: faster, however it seems to miss a few packest at the end when >1  - FIXME)
#define BYTES_PER_PACKET       4 

//###### Bit masks of PetLink packets ######
//### Tag 0: event ###
#define EVENT_MASK             0b10000000
#define EVENT_VALUE            0b00000000 

#define DELAY_MASK                          0b11000000
#define DELAY_VALUE                         0b00000000 

#define PROMPT_MASK                         0b11000000
#define PROMPT_VALUE                        0b01000000 

#define BIN_ADDRESS_DATABITS                             0x3FFFFFFF

//### Tag 1: time ###
#define TIME_MASK              0b11000000 
#define TIME_VALUE             0b10000000 

#define ELAPSED_TIME_MASK                   0b11100000 
#define ELAPSED_TIME_VALUE                  0b10000000 

#define DEADTIME_MASK                      0b11100000
#define DEADTIME_VALUE                     0b10100000

#define TIME_DATABITS                                    0x1FFFFFFF

//### Tag 2: motion ###
#define MOTION_MASK            0b11100000
#define MOTION_VALUE           0b11000000 

#define DETECTOR_ROT_MASK                   0b11111111
#define DETECTOR_ROT_VALUE                  0b11000000

#define HEAD_A_RADIAL_POS_MASK              0b11111111
#define HEAD_A_RADIAL_POS_VALUE             0b11000001

#define HEAD_B_RADIAL_POS_MASK              0b11111111
#define HEAD_B_RADIAL_POS_VALUE             0b11000010

#define VERTICAL_BED_POS_MASK               0b11111111
#define VERTICAL_BED_POS_VALUE              0b11000011

#define HORIZONTAL_BED_POS_MASK             0b11111111
#define HORIZONTAL_BED_POS_VALUE            0b11000100

#define GANTRY_LEFT_RIGHT_POS_MASK          0b11111111
#define GANTRY_LEFT_RIGHT_POS_VALUE         0b11000101

#define SOURCE_AXIAL_POS_AND_ROT_MASK       0b11111111
#define SOURCE_AXIAL_POS_AND_ROT_VALUE      0b11000110

#define HRRT_SINGLE_PHOTON_SOURCE_POS_MASK  0b11111111
#define HRRT_SINGLE_PHOTON_SOURCE_POS_VALUE 0b11000111

//### Tag 3: monitoring ###
#define MONITORING_MASK        0b11110000
#define MONITORING_VALUE       0b11100000 

#define GATING0_MASK                         0b11111111
#define GATING0_VALUE                        0b11100000
#define GATING1_MASK                         0b11111111
#define GATING1_VALUE                        0b11100001
#define GATING2_MASK                         0b11111111
#define GATING2_VALUE                        0b11100010
#define GATING3_MASK                         0b11111111
#define GATING3_VALUE                        0b11100011
#define GATING4_MASK                         0b11111111
#define GATING4_VALUE                        0b11100100
#define GATING5_MASK                         0b11111111
#define GATING5_VALUE                        0b11100101
#define GATING6_MASK                         0b11111111
#define GATING6_VALUE                        0b11100110
#define GATING7_MASK                         0b11111111
#define GATING7_VALUE                        0b11100111

#define GATING_DATABITS                                0x0000FFFF

#define MOTION_TRACKING_MASK                0b11111000
#define MOTION_TRACKING_VALUE               0b11101000

#define MOTION_TRACKING_DATABITS                       0x07FFFFFF

//### Tag 4: control ###
#define CONTROL_MASK           0b11110000
#define CONTROL_VALUE          0b11110000 

#define CONTROL_PET_MASK       0b10000000
#define CONTROL_PET_VALUE      0b00000000

#define CONTROL_MR_MASK        0b11110000
#define CONTROL_MR_VALUE       0b10000000

#define CONTROL_PET_DATABITS                           0x00007FFF 
#define CONTROL_MR_DATABITS                            0x00000FFF 
#define CONTROL_OTHER_DATABITS                         0x00000FFF 

//FIXME: checksum of control packets is currently ignored, implement checksum verification 

//###### Packet identifiers (for internal use - in this software library) ######

#define TAG_UNKNOWN                               0
#define TAG_EVENT                                 1
#define TAG_TIME                                  2
#define TAG_MOTION                                3
#define TAG_MONITORING                            4
#define TAG_CONTROL                               5

#define TYPE_UNKNOWN                                 0 

#define TYPE_EVENT_PROMPT                            1 
#define TYPE_EVENT_DELAY                             2

#define TYPE_TIME_ELAPSED                            3 
#define TYPE_TIME_DEADTIME                           4

#define TYPE_MOTION_DETECTOR_ROT                     5
#define TYPE_MOTION_HEAD_A_RADIAL_POS                6
#define TYPE_MOTION_HEAD_B_RADIAL_POS                7
#define TYPE_MOTION_VERTICAL_BED_POS                 8
#define TYPE_MOTION_HORIZONTAL_BED_POS               9 
#define TYPE_MOTION_GANTRY_LEFT_RIGHT_POS            10 
#define TYPE_MOTION_SOURCE_AXIAL_POS_AND_ROT         11 
#define TYPE_MOTION_HRRT_SINGLE_PHOTON_SOURCE_POS    12 

#define TYPE_MONITORING_GATING0                      13 
#define TYPE_MONITORING_GATING1                      14 
#define TYPE_MONITORING_GATING2                      15 
#define TYPE_MONITORING_GATING3                      16 
#define TYPE_MONITORING_GATING4                      17 
#define TYPE_MONITORING_GATING5                      18 
#define TYPE_MONITORING_GATING6                      19 
#define TYPE_MONITORING_GATING7                      20 
#define TYPE_MONITORING_MOTION_TRACKING              21 

#define TYPE_CONTROL_PET                             22 
#define TYPE_CONTROL_MR                              23 
#define TYPE_CONTROL_OTHER                           24 

typedef struct Packet {
   char buffer[4]; 
   int tag; 
   int type;
   int bin_index; // used if tag indicates event 
   int time_ms;   // used if tag indicates time 
   int data;      // used for other tags and types 
} Packet;


int clear_packet(Packet *packet)
{
   packet->tag             = TAG_UNKNOWN;
   packet->type            = TYPE_UNKNOWN; 
   packet->bin_index       = 0; 
   packet->time_ms         = 0;
   packet->data            = 0;
   return STATUS_SUCCESS;
}


/* 
Print n as a binary number. Utility function for debugging. 
*/
void printbits(int n) 
{
    unsigned int i, step;

    if (0 == n)  /* For simplicity's sake, I treat 0 as a special case*/
    {
        printf("0000");
        return;
    }

    i = 1<<(sizeof(n) * 8 - 1);

    step = -1; /* Only print the relevant digits */
    step >>= 4; /* In groups of 4 */
    while (step >= n) 
    {
        i >>= 4;
        step >>= 4;
    }

    /* At this point, i is the smallest power of two larger or equal to n */
    while (i > 0) 
    {
        if (n & i)
            printf("1");
        else
            printf("0");
        i >>= 1;
    }
    printf("\n");
}



int decode_packet(Packet* packet)
{
    // Uncomment the following lines to print packet information for debugging. 
    //int *int_p; 
    //int_p = (int*) packet->buffer; 
    //printbits(*int_p); 

    //###### Detect the type of packet ######
    //detect bits configuration: check if the 'binary and' of PACKET and MASK equals VALUE: 
    if ( !((packet->buffer[3] & EVENT_MASK) - EVENT_VALUE) ) {
        packet->tag = TAG_EVENT;  
    }
    else if ( !((packet->buffer[3] & TIME_MASK) - TIME_VALUE) ) {        
        packet->tag = TAG_TIME; 
    }
    else if ( !((packet->buffer[3] & MOTION_MASK) - MOTION_VALUE) ) {        
        packet->tag = TAG_MOTION; 
    }
    else if ( !((packet->buffer[3] & MONITORING_MASK) - MONITORING_VALUE) ) {        
        packet->tag = TAG_MONITORING; 
    }
    else if ( !((packet->buffer[3] & CONTROL_MASK) - CONTROL_VALUE) ) {        
        packet->tag = TAG_CONTROL; 
    }
    else {
        packet->tag = TAG_UNKNOWN; 
    }

   //###### Read the content of the packet ######
   //## Event ##
   if (packet->tag == TAG_EVENT) {
       packet->bin_index = ( *(int*)(packet->buffer) & BIN_ADDRESS_DATABITS); 
       if ( !((packet->buffer[3] & PROMPT_MASK) - PROMPT_VALUE) ) {   
           packet->type = TYPE_EVENT_PROMPT; 
       } 
       else if ( !((packet->buffer[3] & DELAY_MASK) - DELAY_VALUE) ) {        
           packet->type = TYPE_EVENT_DELAY; 
       } 
       else {
           packet->type = TYPE_UNKNOWN;            
       }
   }
   //## Time ## 
   else if (packet->tag == TAG_TIME) {
       packet->time_ms = ( *(int*)(packet->buffer) & TIME_DATABITS); 
       if ( !((packet->buffer[3] & ELAPSED_TIME_MASK) - ELAPSED_TIME_VALUE) ) {  
           packet->type = TYPE_TIME_ELAPSED; 
       }
       else if ( !((packet->buffer[3] & DEADTIME_MASK) - DEADTIME_VALUE) ) {  
           packet->type = TYPE_TIME_DEADTIME; 
       }
       else {
           packet->type = TYPE_UNKNOWN; 
       }
   }


   //## Motion ## 
   else if (packet->tag == TAG_MOTION) {
       //FIXME: implement 
   }

   //## Monitoring ## 
   else if (packet->tag == TAG_MONITORING) {
       if ( !((packet->buffer[3] & GATING0_MASK) - GATING0_VALUE) ) { 
           packet->type = TYPE_MONITORING_GATING0; 
           packet->data = ( *(int*)(packet->buffer) & GATING_DATABITS); 
       }
       else if ( !((packet->buffer[3] & GATING1_MASK) - GATING1_VALUE) ) { 
           packet->type = TYPE_MONITORING_GATING1; 
           packet->data = ( *(int*)(packet->buffer) & GATING_DATABITS); 
       }
       else if ( !((packet->buffer[3] & GATING2_MASK) - GATING2_VALUE) ) { 
           packet->type = TYPE_MONITORING_GATING2; 
           packet->data = ( *(int*)(packet->buffer) & GATING_DATABITS); 
       }           
       else if ( !((packet->buffer[3] & GATING3_MASK) - GATING3_VALUE) ) { 
           packet->type = TYPE_MONITORING_GATING3; 
           packet->data = ( *(int*)(packet->buffer) & GATING_DATABITS); 
       }           
       else if ( !((packet->buffer[3] & GATING4_MASK) - GATING4_VALUE) ) { 
           packet->type = TYPE_MONITORING_GATING4; 
           packet->data = ( *(int*)(packet->buffer) & GATING_DATABITS); 
       }           
       else if ( !((packet->buffer[3] & GATING5_MASK) - GATING5_VALUE) ) { 
           packet->type = TYPE_MONITORING_GATING5; 
           packet->data = ( *(int*)(packet->buffer) & GATING_DATABITS); 
       }           
       else if ( !((packet->buffer[3] & GATING6_MASK) - GATING6_VALUE) ) { 
           packet->type = TYPE_MONITORING_GATING6; 
           packet->data = ( *(int*)(packet->buffer) & GATING_DATABITS); 
       }           
       else if ( !((packet->buffer[3] & GATING7_MASK) - GATING7_VALUE) ) { 
           packet->type = TYPE_MONITORING_GATING7; 
           packet->data = ( *(int*)(packet->buffer) & GATING_DATABITS); 
       }
       else if ( !((packet->buffer[3] & MOTION_TRACKING_MASK) - MOTION_TRACKING_VALUE) ) {      
           packet->type = TYPE_MONITORING_MOTION_TRACKING; 
           packet->data = ( *(int*)(packet->buffer) & MOTION_TRACKING_DATABITS); 
       }
       else {
           packet->type = TYPE_UNKNOWN; 
       }
   }

   //## Control ## 
   else if (packet->tag == TAG_CONTROL) {
       // FIXME: checksum currently ignored: verify checksum 
       if ( !((packet->buffer[1] & CONTROL_PET_MASK) - CONTROL_PET_VALUE) ) { 
           packet->type = TYPE_CONTROL_PET; 
           packet->data = ( *(int*)(packet->buffer) & CONTROL_PET_DATABITS); 
       }
       else if ( !((packet->buffer[1] & CONTROL_MR_MASK) - CONTROL_MR_VALUE) ) { 
           packet->type = TYPE_CONTROL_MR; 
           packet->data = ( *(int*)(packet->buffer) & CONTROL_MR_DATABITS); 
       }   
       else {
           packet->type = TYPE_CONTROL_OTHER; 
           packet->data = ( *(int*)(packet->buffer) & CONTROL_OTHER_DATABITS);        
       }       
   }

   return STATUS_SUCCESS; 
}


int read_packets(FILE *stream, char *buffer, int64_t number_of_packets)
{
    if( fread(buffer, BYTES_PER_PACKET, number_of_packets, stream ) != number_of_packets)   
        return STATUS_IO_ERROR;  
    return STATUS_SUCCESS; 
}


int read_packets_int(FILE *stream, char *buffer, int number_of_packets)
{
    if( fread(buffer, BYTES_PER_PACKET, (size_t) number_of_packets, stream ) != number_of_packets)   
        return STATUS_IO_ERROR;  
    return STATUS_SUCCESS; 
}


int read_packet(FILE *stream, Packet* packet) 
{
    if( fread(packet->buffer,4,1,stream) != 1) 
        return STATUS_IO_ERROR; 
    return STATUS_SUCCESS; 
}


int read_and_decode_packet(FILE *stream, Packet* packet)
{
    if( read_packet(stream, packet) != STATUS_SUCCESS) 
        return STATUS_IO_ERROR; 
    return decode_packet(packet);
}






/*
Information about petlink32 listmode data. 
*/
extern int petlink32_info(char *filename, int64_t *n_packets, unsigned int *time_first, unsigned int *time_last, unsigned int *n_unknown, unsigned int *n_prompt, unsigned int *n_delayed, unsigned int *n_elapsedtime, unsigned int *n_deadtime, unsigned int *n_motion, unsigned int *n_gating0, unsigned int *n_gating1, unsigned int *n_gating2, unsigned int *n_gating3, unsigned int *n_gating4, unsigned int *n_gating5, unsigned int *n_gating6, unsigned int *n_gating7, unsigned int *n_tracking, unsigned int *n_control_PET, unsigned int *n_control_MR, unsigned int *n_control_OTHER)
{
    int status = STATUS_SUCCESS; 
    unsigned int buffer_size_batch = 1;  //buffer_size_batch
    
    // make a buffer to store one batch of packets 
    char batch_buffer[buffer_size_batch*BYTES_PER_PACKET]; 

    // Open the petlink 32 bit listmode file
    FILE *fid; 
    fid=fopen(filename, "rb");
    if (fid == NULL) {
        fprintf(stderr,"Failed to open listmode file. \n");
        status = STATUS_IO_ERROR; 
        return status; 
    }

    // Read and decode packets 
    int i, j; 
    
    int64_t N_packets  = *n_packets; 
    *n_unknown = 0; 
    *n_prompt = 0; 
    *n_delayed = 0;
    *n_elapsedtime = 0; 
    *n_deadtime = 0; 
    *n_motion = 0; 
    *n_gating0 = 0; 
    *n_gating1 = 0; 
    *n_gating2 = 0; 
    *n_gating3 = 0; 
    *n_gating4 = 0; 
    *n_gating5 = 0; 
    *n_gating6 = 0; 
    *n_gating7 = 0; 
    *n_tracking = 0; 
    *n_control_PET = 0; 
    *n_control_MR  = 0; 
    *n_control_OTHER = 0; 
    
    Packet packet;

    int64_t N_batches = floor(N_packets / buffer_size_batch) + 1; 
    int64_t n_packets_last_batch = N_packets - (N_batches-1)*buffer_size_batch; 
    int64_t n_packets_batch; 

    unsigned int done = 0; 
    unsigned int first_time_packet=1; 

    for (j = 0; j < N_batches; j++) {
        if (done) {
            break; 
        }
        if (j==N_batches-1) {
            n_packets_batch = n_packets_last_batch; 
        }
        else {
            n_packets_batch = buffer_size_batch; 
        }
        status = read_packets(fid, batch_buffer, n_packets_batch); 
        if (status!=STATUS_SUCCESS) {
            fprintf(stderr,"Problem reading the listmode data.\n"); 
            //return status; 
            status = STATUS_SUCCESS; 
            done = 1; 
            break; 
        }
        for (i = 0; i < n_packets_batch; i++) { 
            if (done) {
                break; 
            }
            //clear_packet(&packet);
            memcpy ( packet.buffer, batch_buffer+i*BYTES_PER_PACKET, BYTES_PER_PACKET );
            //packet->buffer = batch_buffer[]; 
            status = decode_packet(&packet);
            if (status!=STATUS_SUCCESS) {
                fprintf(stderr,"Problem decoding the listmode data.\n"); 
                return status; 
            }    
            if (packet.type == TYPE_EVENT_PROMPT) {
                *n_prompt = *n_prompt + 1; } 
            else if (packet.type == TYPE_EVENT_DELAY) {
                *n_delayed = *n_delayed + 1; }
            else if (packet.type == TYPE_TIME_ELAPSED) {
                //fprintf(stderr,"Elapsed time: %d msec\n",packet.time_ms);
                if (first_time_packet==1) {
                    first_time_packet = 0; 
                    *time_first = packet.time_ms; 
                }
                else {
                    *time_last = packet.time_ms; 
                }
                *n_elapsedtime = *n_elapsedtime + 1; } 
            else if (packet.type == TYPE_TIME_DEADTIME) {
                *n_deadtime = *n_deadtime + 1; } 
            else if (packet.tag == TAG_MOTION) {     // notice: just check the tag, no specific type for motion (this could be extended)
                *n_motion = *n_motion + 1; }         
            else if (packet.type == TYPE_MONITORING_GATING0) {
                *n_gating0 = *n_gating0 + 1; } 
            else if (packet.type == TYPE_MONITORING_GATING1) {
                *n_gating1 = *n_gating1 + 1; 
                } 
            else if (packet.type == TYPE_MONITORING_GATING2) {
                *n_gating2 = *n_gating2 + 1; } 
            else if (packet.type == TYPE_MONITORING_GATING3) {
                *n_gating3 = *n_gating3 + 1; } 
            else if (packet.type == TYPE_MONITORING_GATING4) {
                *n_gating4 = *n_gating4 + 1; } 
            else if (packet.type == TYPE_MONITORING_GATING5) {
                *n_gating5 = *n_gating5 + 1; } 
            else if (packet.type == TYPE_MONITORING_GATING6) {
                *n_gating6 = *n_gating6 + 1; }             
            else if (packet.type == TYPE_MONITORING_GATING7) {
                *n_gating7 = *n_gating7 + 1; 
                }                 
            else if (packet.type == TYPE_MONITORING_MOTION_TRACKING) {
                *n_tracking = *n_tracking + 1; } 
            else if (packet.type == TYPE_CONTROL_PET) {
                *n_control_PET = *n_control_PET + 1; 
                fprintf(stderr,"Control packet (PET):   %d \n",packet.data); //FIXME: delete this line
                } 
            else if (packet.type == TYPE_CONTROL_MR) {
                *n_control_MR = *n_control_MR + 1; 
                fprintf(stderr,"Control packet (MR):    %d \n",packet.data); //FIXME: delete this line
                } 
            else if (packet.type == TYPE_CONTROL_OTHER) {
                *n_control_OTHER = *n_control_OTHER + 1; 
                fprintf(stderr,"Control packet (OTHER): %d \n",packet.data); //FIXME: delete this line
                } 
            else {
                *n_unknown = *n_unknown + 1; } 
            }
        }

    // Close file 
    fclose(fid); 
    return status; 
}



/*
Converts petlink32 listmode data to a static sinogram.
*/
extern int petlink32_to_static_sinogram_mMR(char *filename, int64_t *n_packets, int *n_radial_bins, int *n_angles, int *n_sinograms, int *sinogram, int *n_unknown, int *n_prompt, int *n_delayed, int *n_elapsedtime, int *n_deadtime, int *n_motion, int *n_gating0, int *n_gating1, int *n_gating2, int *n_gating3, int *n_gating4, int *n_gating5, int *n_gating6, int *n_gating7, int *n_tracking, int *n_control_PET, int *n_control_MR, int *n_control_OTHER)
{
    int status = STATUS_SUCCESS; 
    
    // make a buffer to store one batch of packets 
    char batch_buffer[BUFFER_SIZE_BATCH*BYTES_PER_PACKET]; 
    
    // Open the petlink 32 bit listmode file
    FILE *fid; 
    fid=fopen(filename, "rb");
    if (fid == NULL) {
        fprintf(stderr,"Failed to open listmode file. \n");
        status = STATUS_IO_ERROR; 
        return status; 
    }

    // Read and decode packets 
    int i, j; 
    int bin_index, sinogram_index, angle_index, radial_index, tmp_index; 
    
    int N_radial_bins  = *n_radial_bins; // inputs
    int N_angles       = *n_angles; 
    int N_sinograms    = *n_sinograms; 
    int64_t N_packets  = *n_packets; 

    *n_unknown = 0; 
    *n_prompt = 0; 
    *n_delayed = 0;
    *n_elapsedtime = 0; 
    *n_deadtime = 0; 
    *n_motion = 0; 
    *n_gating0 = 0; 
    *n_gating1 = 0; 
    *n_gating2 = 0; 
    *n_gating3 = 0; 
    *n_gating4 = 0; 
    *n_gating5 = 0; 
    *n_gating6 = 0; 
    *n_gating7 = 0; 
    *n_tracking = 0; 
    *n_control_PET = 0; 
    *n_control_MR  = 0; 
    *n_control_OTHER = 0; 
    
    int N = N_radial_bins * N_angles;      // just precopute this product 
    Packet packet;

    int64_t N_batches = floor(N_packets / BUFFER_SIZE_BATCH) + 1; 
    int64_t n_packets_last_batch = N_packets - (N_batches-1)*BUFFER_SIZE_BATCH; 
    int64_t n_packets_batch; 
    
    for (j = 0; j < N_batches; j++) {
        if (j==N_batches-1) {
            n_packets_batch = n_packets_last_batch; 
        }
        else {
            n_packets_batch = BUFFER_SIZE_BATCH; 
        }
        status = read_packets(fid, batch_buffer, n_packets_batch); 
        if (status!=STATUS_SUCCESS) {
            fprintf(stderr,"Problem reading the listmode data.\n"); 
            fclose(fid); 
            return status; 
        }
        for (i = 0; i < n_packets_batch; i++) { 
            //clear_packet(&packet);
            memcpy ( packet.buffer, batch_buffer+i*BYTES_PER_PACKET, BYTES_PER_PACKET );
            //packet->buffer = batch_buffer[]; 
            status = decode_packet(&packet);
            if (status!=STATUS_SUCCESS) {
                fprintf(stderr,"Problem decoding the listmode data.\n"); 
                fclose(fid); 
                return status; 
            }    
            if (packet.type == TYPE_EVENT_PROMPT) {
                *n_prompt = *n_prompt + 1; } 
            else if (packet.type == TYPE_EVENT_DELAY) {
                *n_delayed = *n_delayed + 1; }
            else if (packet.type == TYPE_TIME_ELAPSED) {
                //fprintf(stderr,"Elapsed time: %d msec\n",packet.time_ms);
                *n_elapsedtime = *n_elapsedtime + 1; } 
            else if (packet.type == TYPE_TIME_DEADTIME) {
                *n_deadtime = *n_deadtime + 1; } 
            else if (packet.tag == TAG_MOTION) {     // notice: just check the tag, no specific type for motion (this could be extended)
                *n_motion = *n_motion + 1; }         
            else if (packet.type == TYPE_MONITORING_GATING0) {
                *n_gating0 = *n_gating0 + 1; } 
            else if (packet.type == TYPE_MONITORING_GATING1) {
                *n_gating1 = *n_gating1 + 1; } 
            else if (packet.type == TYPE_MONITORING_GATING2) {
                *n_gating2 = *n_gating2 + 1; } 
            else if (packet.type == TYPE_MONITORING_GATING3) {
                *n_gating3 = *n_gating3 + 1; } 
            else if (packet.type == TYPE_MONITORING_GATING4) {
                *n_gating4 = *n_gating4 + 1; } 
            else if (packet.type == TYPE_MONITORING_GATING5) {
                *n_gating5 = *n_gating5 + 1; } 
            else if (packet.type == TYPE_MONITORING_GATING6) {
                *n_gating6 = *n_gating6 + 1; }             
            else if (packet.type == TYPE_MONITORING_GATING7) {
                *n_gating7 = *n_gating7 + 1; }                 
            else if (packet.type == TYPE_MONITORING_MOTION_TRACKING) {
                *n_tracking = *n_tracking + 1; } 
            else if (packet.type == TYPE_CONTROL_PET) {
                *n_control_PET = *n_control_PET + 1; } 
            else if (packet.type == TYPE_CONTROL_MR) {
                *n_control_MR = *n_control_MR + 1; } 
            else if (packet.type == TYPE_CONTROL_OTHER) {
                *n_control_OTHER = *n_control_OTHER + 1; } 
            else { *n_unknown = *n_unknown + 1; } 
            
            if (packet.type == TYPE_EVENT_PROMPT) { //FIXME: also make a sinogram for the delayed events 
                bin_index        = packet.bin_index; 
                sinogram_index   = floor(bin_index / N); 
                angle_index      = floor((bin_index-sinogram_index*N)/N_radial_bins ); 
                radial_index     = bin_index-sinogram_index*N-angle_index*N_radial_bins; 
                // make sure that they don't exceed the size of the sinogram: 
                // .. FIXME: see line above
                tmp_index = radial_index*N_angles*N_sinograms+angle_index*N_sinograms+sinogram_index;
                sinogram[(int)tmp_index] = sinogram[(int)tmp_index]+1; 
            }
        }
    }

    // Close file 
    fclose(fid); 
    return status; 
}










typedef struct OffsetsMatrix{
    unsigned int N_axial;         // number of detector planes, axial
    unsigned int N_azimuthal;     // number of detector planes, azimuthal 
    unsigned int *matrix;         // [N_axial x N_azimuthal] offsets matrix: element (i,j) reports where the data for detector plane (i,j) is. 
                                  // The integer is an offset of the projection counts array (in array elements, not bytes). 
} OffsetsMatrix; 

typedef struct ProjectionBinning{ 
    unsigned int N_axial;         // number of detector planes, axial
    unsigned int N_azimuthal;     // number of detector planes, azimuthal 
    float* angles_axial;          // array angles between adjacent planes, axial - radians
    float* angles_azimuthal;      // array angles between adjacent planes, azimuthal - radians    
    float size_u;                 // size of the detector plane, axial
    float size_v;                 // size of the detector plane, longitudinal 
    float N_u;                    // number of bins of the detector plane, axial
    float N_v;                    // number of bins of the detector plane, longitudinal  
} ProjectionBinning; 

typedef struct VolumeDescriptor{ 
    unsigned int N_x; 
    unsigned int N_y; 
    unsigned int N_z;     
    float V_x;     
    float V_y;     
    float V_z;         
    float d_x; 
    float d_y; 
    float d_z;     
} VolumeDescriptor; 

typedef struct Volume{
    VolumeDescriptor *descriptor; 
    float            *data; 
} Volume; 

typedef struct StaticProjectionLayout{
    ProjectionBinning *binning; 
    OffsetsMatrix     *offsets; 
} StaticProjectionLayout; 

typedef struct StaticProjection{
    StaticProjectionLayout *layout; 
    float                  *counts; 
    unsigned short         *locations; 
    float                  time_start; 
    float                  time_end; 
    int64_t                N_counts; 
    unsigned int           N_locations; 
    float                  compression_ratio; 
    float                  listmode_loss; 
} StaticProjection; 

typedef struct DynamicProjection{
    unsigned int     N_time_bins; 
    int64_t          N_counts; 
    unsigned int     N_locations; 
    float            compression_ratio; 
    float            dynamic_inflation; 
    float            listmode_loss;    
    unsigned int     time_start; 
    unsigned int     time_end; 
    StaticProjection **static_projections; 
    StaticProjection *global_static_projection; 
} DynamicProjection; 

typedef struct ListModeStats{
    int64_t      n_packets;
    unsigned int n_unknown;
    unsigned int n_prompt;
    unsigned int n_delayed;
    unsigned int n_elapsedtime;
    unsigned int n_deadtime;
    unsigned int n_motion;
    unsigned int n_gating0;
    unsigned int n_gating1;    
    unsigned int n_gating2;    
    unsigned int n_gating3;    
    unsigned int n_gating4;    
    unsigned int n_gating5;    
    unsigned int n_gating6;    
    unsigned int n_gating7;    
    unsigned int n_tracking;
    unsigned int n_control_PET; 
    unsigned int n_control_MR;     
    unsigned int n_control_OTHER;      
} ListModeStats;

// Line-of-response 
typedef struct LOR{
    float x0; 
    float y0; 
    float z0;     
    float x1; 
    float y1; 
    float z1;         
} LOR; 






OffsetsMatrix *instantiate_offsets(unsigned int N_axial, unsigned int N_azimuthal) 
{
    OffsetsMatrix *offsets = (OffsetsMatrix *) malloc(sizeof(OffsetsMatrix)); 
    offsets->N_axial      = N_axial; 
    offsets->N_azimuthal  = N_azimuthal; 
    offsets->matrix       = (unsigned int*) malloc(sizeof(unsigned int)*N_axial*N_azimuthal);  //FIXME: free ! 
    return offsets; 
}


ProjectionBinning *instantiate_binning(unsigned int N_axial, unsigned int N_azimuthal, float* Angles_axial, float* Angles_azimuthal, float Size_u, float Size_v, unsigned int N_u, unsigned int N_v)
{
    ProjectionBinning *binning = (ProjectionBinning *) malloc(sizeof(ProjectionBinning ));
    binning->N_axial                = N_axial; 
    binning->N_azimuthal            = N_azimuthal; 
    binning->angles_axial           = Angles_axial;
    binning->angles_azimuthal       = Angles_azimuthal; 
    binning->size_u                 = Size_u; 
    binning->size_v                 = Size_v; 
    binning->N_u                    = N_u; 
    binning->N_v                    = N_v; 
    return binning; 
}


StaticProjectionLayout *instantiate_layout(OffsetsMatrix *offsets, ProjectionBinning *binning)
{
    StaticProjectionLayout *layout = (StaticProjectionLayout *) malloc(sizeof(StaticProjectionLayout)); 
    layout->offsets  = offsets; 
    layout->binning  = binning; 
    return layout; 
}


StaticProjection *instantiate_static_projection(unsigned int time_start, unsigned int time_end, int64_t N_counts, unsigned int N_locations, float compression_ratio, float listmode_loss, float *counts, unsigned short *locations, StaticProjectionLayout *layout)
{
    StaticProjection *static_projection = (StaticProjection *) malloc(sizeof(StaticProjection)); 
    static_projection->layout     = layout; 
    static_projection->counts     = counts;
    static_projection->locations  = locations;    
    static_projection->time_start = time_start; 
    static_projection->time_end   = time_end; 
    static_projection->N_counts          = N_counts;   
    static_projection->N_locations       = N_locations; 
    static_projection->compression_ratio = compression_ratio;    
    static_projection->listmode_loss     = listmode_loss; 
    return static_projection; 
}


DynamicProjection *instantiate_dynamic_projection(unsigned int N_time_bins)
{
    DynamicProjection *dynamic_projection = (DynamicProjection *) malloc(sizeof(DynamicProjection)); 
    dynamic_projection->N_time_bins = N_time_bins; 

    // Initialise the array of static projections, one per time bin 
    StaticProjection *static_projections[MAX_TIME_BINS]; 
    int i;    
    for (i=0; i<MAX_TIME_BINS;i++){
        static_projections[i] = NULL; 
    }
    dynamic_projection->static_projections = static_projections; 
    
    // Initialise the global static projection 
    dynamic_projection->global_static_projection = NULL; 
    
    return dynamic_projection; 
}


void clear_stats(ListModeStats *stats)
{
    stats->n_packets=0;
    stats->n_unknown=0;
    stats->n_prompt=0;
    stats->n_delayed=0;
    stats->n_elapsedtime=0;
    stats->n_deadtime=0;
    stats->n_motion=0;
    stats->n_gating0=0;
    stats->n_gating1=0;    
    stats->n_gating2=0;
    stats->n_gating3=0;    
    stats->n_gating4=0;
    stats->n_gating5=0;    
    stats->n_gating6=0;
    stats->n_gating7=0;    
    stats->n_tracking=0;
    stats->n_control_PET =0; 
    stats->n_control_MR =0; 
    stats->n_control_OTHER =0;     
}

ListModeStats *instantiate_stats()
{
    ListModeStats *stats = (ListModeStats *) malloc(sizeof(ListModeStats)); 
    clear_stats(stats); 
    return stats; 
}




typedef struct GatingData{
    unsigned int           n_events; 
    unsigned int           n_gating0;
    unsigned int           n_gating1; 
    unsigned int           n_gating2; 
    unsigned int           n_gating3; 
    unsigned int           n_gating4; 
    unsigned int           n_gating5; 
    unsigned int           n_gating6; 
    unsigned int           n_gating7;  
    unsigned int           *type; 
    unsigned int           *time; 
    unsigned int           *payload; 
    unsigned int           search_time_start;
    unsigned int           search_time_end; 
    unsigned int           initialized; 
} GatingData; 


void clear_gating_data(GatingData *gating)
{
    if(gating->initialized!=0) {
//        free(gating->type);
//        free(gating->time);
//        free(gating->payload);
    	}
    gating->initialized=0; 
    gating->n_events=0;
    gating->n_gating0=0;
    gating->n_gating1=0; 
    gating->n_gating2=0;
    gating->n_gating3=0;
    gating->n_gating4=0;
    gating->n_gating5=0;
    gating->n_gating6=0;
    gating->n_gating7=0;
    gating->search_time_start=0;
    gating->search_time_end=0;
}

GatingData *instantiate_gating(unsigned int max_n_events)
{
    GatingData *gating = (GatingData *) malloc(sizeof(GatingData)); 
    clear_gating_data(gating); 
    gating->type = (unsigned int*) malloc(sizeof(unsigned int)*max_n_events);
    gating->time = (unsigned int*) malloc(sizeof(unsigned int)*max_n_events);
    gating->payload = (unsigned int*) malloc(sizeof(unsigned int)*max_n_events);
    gating->initialized=1;
    return gating; 
}


/*
Global data structure, constructor and destructor 
*/
typedef struct GlobalObject {
    DynamicProjection *dynamic_projection; 
    DynamicProjection *dynamic_projection_delay; 
    ListModeStats     *stats; 
    GatingData        *gating; 
    unsigned int      has_projection;
    unsigned int      has_gating_data;  
} GlobalObject; 

static GlobalObject globalobject; 

int initialise_globalobject(void) 
{
    globalobject.has_projection  = 0;
    globalobject.has_gating_data = 0;
    return 0;
}

initialise_globalobject(); 


int deallocate_globalobject(void)
{    
    if (globalobject.has_gating_data) {
        fprintf(stderr,"Freeing gating data..  \n"); 
        if (globalobject.gating->initialized) {
            free(globalobject.gating->type); 
            free(globalobject.gating->time);
            free(globalobject.gating->payload);
            }
        free(globalobject.gating);
        }
    
    if (globalobject.has_projection) {
        fprintf(stderr,"Freeing stats.. \n");
        free(globalobject.stats); 
    
        // Free dynamic projection
        fprintf(stderr,"Freeing dynamic projection.. \n");
        int t; 
        for (t=0; t<globalobject.dynamic_projection->N_time_bins; t++) {
            // Free static projections 
                if (globalobject.dynamic_projection->static_projections[t] != NULL) {
                if (globalobject.dynamic_projection->static_projections[t]->layout != NULL) { 
                    if (globalobject.dynamic_projection->static_projections[t]->layout->offsets != NULL) { 
                        if (globalobject.dynamic_projection->static_projections[t]->layout->offsets->matrix != NULL) {
                            free(globalobject.dynamic_projection->static_projections[t]->layout->offsets->matrix); 
                        }
                        free(globalobject.dynamic_projection->static_projections[t]->layout->offsets); 
                    }
                    if (globalobject.dynamic_projection->static_projections[t]->layout->binning != NULL) { 
                        free(globalobject.dynamic_projection->static_projections[t]->layout->binning);   
                    }
                    free(globalobject.dynamic_projection->static_projections[t]->layout);     
                }
                if (globalobject.dynamic_projection->static_projections[t]->counts != NULL) {
                    free(globalobject.dynamic_projection->static_projections[t]->counts); 
                }
                if (globalobject.dynamic_projection->static_projections[t]->locations != NULL) {
                    free(globalobject.dynamic_projection->static_projections[t]->locations); 
                }             
                free(globalobject.dynamic_projection->static_projections[t]); 
            }
        }
        // Free the global static projection 
        fprintf(stderr,"Freeing global static projection.. \n");
        if (globalobject.dynamic_projection->global_static_projection != NULL) {
            if (globalobject.dynamic_projection->global_static_projection->layout != NULL) { 
                if (globalobject.dynamic_projection->global_static_projection->layout->offsets != NULL) { 
                    if (globalobject.dynamic_projection->global_static_projection->layout->offsets->matrix != NULL) {
                        free(globalobject.dynamic_projection->global_static_projection->layout->offsets->matrix); 
                    }
                    free(globalobject.dynamic_projection->global_static_projection->layout->offsets); 
                }
                if (globalobject.dynamic_projection->global_static_projection->layout->binning != NULL) { 
                    free(globalobject.dynamic_projection->global_static_projection->layout->binning);   
                }
                free(globalobject.dynamic_projection->global_static_projection->layout);     
            }
            if (globalobject.dynamic_projection->global_static_projection->counts != NULL) {
                free(globalobject.dynamic_projection->global_static_projection->counts); 
            }
            if (globalobject.dynamic_projection->global_static_projection->locations != NULL) {
                free(globalobject.dynamic_projection->global_static_projection->locations); 
            }             
            free(globalobject.dynamic_projection->global_static_projection); 
        }

        free(globalobject.dynamic_projection); 
    
        /* FIXME: this comment causes a memory leak. If uncommented, however, there is a segmentation fault. Why? 
        // Free dynamic projection delayed and static projection delayed 
        // Free dynamic projection
        fprintf(stderr,"Freeing dynamic projection delays.. \n");
        for (t=0; t<globalobject.dynamic_projection_delay->N_time_bins; t++) {
            // Free static projections 
            if (globalobject.dynamic_projection_delay->static_projections[t] != NULL) {
                if (globalobject.dynamic_projection_delay->static_projections[t]->layout != NULL) { 
                    if (globalobject.dynamic_projection_delay->static_projections[t]->layout->offsets != NULL) { 
                        if (globalobject.dynamic_projection_delay->static_projections[t]->layout->offsets->matrix != NULL) {
                            free(globalobject.dynamic_projection_delay->static_projections[t]->layout->offsets->matrix); 
                        }
                        free(globalobject.dynamic_projection_delay->static_projections[t]->layout->offsets); 
                    }
                    if (globalobject.dynamic_projection_delay->static_projections[t]->layout->binning != NULL) { 
                        free(globalobject.dynamic_projection_delay->static_projections[t]->layout->binning);   
                    }
                    free(globalobject.dynamic_projection_delay->static_projections[t]->layout);     
                }
                if (globalobject.dynamic_projection_delay->static_projections[t]->counts != NULL) {
                    free(globalobject.dynamic_projection_delay->static_projections[t]->counts); 
                }
                if (globalobject.dynamic_projection_delay->static_projections[t]->locations != NULL) {
                    free(globalobject.dynamic_projection_delay->static_projections[t]->locations); 
                }             
                free(globalobject.dynamic_projection_delay->static_projections[t]); 
            }
        }
        // Free the global static projection 
        fprintf(stderr,"Freeing global static projection delays .. \n");
        if (globalobject.dynamic_projection_delay->global_static_projection != NULL) {
            if (globalobject.dynamic_projection_delay->global_static_projection->layout != NULL) { 
                if (globalobject.dynamic_projection_delay->global_static_projection->layout->offsets != NULL) { 
                    if (globalobject.dynamic_projection_delay->global_static_projection->layout->offsets->matrix != NULL) {
                        free(globalobject.dynamic_projection_delay->global_static_projection->layout->offsets->matrix); 
                    }
                    free(globalobject.dynamic_projection_delay->global_static_projection->layout->offsets); 
                }
                if (globalobject.dynamic_projection_delay->global_static_projection->layout->binning != NULL) { 
                    free(globalobject.dynamic_projection_delay->global_static_projection->layout->binning);   
                }
                free(globalobject.dynamic_projection_delay->global_static_projection->layout);     
            }
            if (globalobject.dynamic_projection_delay->global_static_projection->counts != NULL) {
                free(globalobject.dynamic_projection_delay->global_static_projection->counts); 
            }
            if (globalobject.dynamic_projection_delay->global_static_projection->locations != NULL) {
                free(globalobject.dynamic_projection_delay->global_static_projection->locations); 
            }             
            free(globalobject.dynamic_projection_delay->global_static_projection); 
        }

        free(globalobject.dynamic_projection_delay); 
        */
    }
    // Reset initialization flag 
    globalobject.has_projection = 0;
    globalobject.has_gating_data = 0; 
        
    fprintf(stderr,"PET Globalobject deallocated.\n");
    return STATUS_SUCCESS; 
}




/*
List-mode index to LOR coordinates
*/
int index_to_LOR_mMR(unsigned int bin_index, unsigned int N_radial_bins, unsigned int N_angles, unsigned int N_projections, LOR *lor)
{
//    unsigned int N = N_radial_bins * N_angles; 
//    unsigned int projection_index = floor(bin_index / N); 
//    unsigned int angle_index    = floor((bin_index-projection_index*N)/N_radial_bins ); 
//    unsigned int radial_index   = bin_index-projection_index*N-angle_index*N_radial_bins; 
    //FIXME: implement (raplaces temporary_hack_index_to_bin() )
    //lor->x0 = ...;    
    //lor->y0 = ...;    
    //lor->z0 = ...;    
    //lor->x1 = ...;    
    //lor->y1 = ...;    
    //lor->z1 = ...;    
    return 0; 
}

/*
LOR coordinates to bin 
*/
int LOR_to_bin(LOR *lor, ProjectionBinning *binning, unsigned int *n_axial, unsigned int *n_azimuthal,unsigned int *n_time, unsigned int *u, unsigned int *v) 
{
    //FIXME: implement (replaces temporary_hack_index_to_bin() )
    *n_axial = 0; 
    *n_azimuthal = 0; 
    *n_time = 0; 
    *u = 0; 
    *v = 0;     
    return 0; 
}



int get_azim_index(unsigned int index_azim, unsigned int N_azim, unsigned int N_average)
{
    int return_value = -1; 
    // array of positions of the azimuthal bin centers: 
    float *bins = (float *) malloc(sizeof(float) * N_azim);
    int i; 
    for (i=0; i<N_azim; i++) {
        bins[i] = (i*120.0)/(N_azim-1);
        }
    // find position closest to the azimuthal index 
    float min_d = 1e10;
    unsigned int min_i = 0; 
    for (i=0; i<N_azim; i++) {
        float d = (bins[i]-index_azim)*(bins[i]-index_azim); 
        if (d < min_d) {
            min_d = d; 
            min_i = i;  
            }
        }
    // check if the distance is less equal to N_average / 2: 
    if ( min_d > (N_average/2.0)*(N_average/2.0) ) {
        return_value = -1; 
        }
    else return_value = min_i; 
//    fprintf(stderr,"\n");
//    for (int i=0; i<N_azim; i++)
//        fprintf(stderr,"%3.2f  ",bins[i]);
//    fprintf(stderr,"\n");
//    fprintf(stderr,"GET_AZIM_INDEX:   index_azim: %d   N_azim: %d   N_average: %d   min_i: %d   min_d: %f   return: %d \n",index_azim, N_azim, N_average,min_i,min_d,return_value);
    return return_value; 
}


unsigned int temporary_hack_index_to_bin(unsigned int bin_index, unsigned int N_radial_bins, unsigned int N_angles, unsigned int N_sinograms, unsigned int N_azimuthal, unsigned int *n_axial, unsigned int *n_azimuthal,unsigned int *n_time, unsigned int *u, unsigned int *v)
{
    unsigned int status=0; 
    unsigned int N = N_radial_bins * N_angles; 
    unsigned int sinogram_index = floor(bin_index / N); 
    unsigned int angle_index    = floor((bin_index-sinogram_index*N)/N_radial_bins ); 
    unsigned int radial_index   = bin_index-sinogram_index*N-angle_index*N_radial_bins; 

    unsigned int tmp_azim = 0; 

    *n_axial = angle_index; 
    *n_time = 0; 
    *u = radial_index;   
    // hack number two: merge 3D sinograms into a 2D sinogram 
    *v = 0;              
    
    unsigned int N_azim    = N_azimuthal; 
    unsigned int N_average = 7; 
    int index_azim = -1; 
    
    if (sinogram_index<64) {
        //fprintf(stderr,"* sinogram_index: %d    v: %d \n",sinogram_index,sinogram_index); 
        *v = sinogram_index; 
        tmp_azim = 60; 
        status=1; 
    }
    
    else {
        status=0; 
        int i=0, c=63, j=0, n=0, d=0;     
        for (i=1; i<60; i++) {
            n++; 
            j = (64-i); 
            c += j; 
                if (sinogram_index <= c) {
                    d=0;
                    break; 
                }
            c += j; 
                if (sinogram_index <= c) {
                    d=1; 
                    break; 
                }
        }
        *v = (j-1-(c-sinogram_index)) + (int)((64-j)/2); 
        if (d==0) {     
            tmp_azim = 60-n; 
        }
        if (d==1) {
            tmp_azim = 60+n; 
        }
    }
    index_azim = get_azim_index(tmp_azim, N_azim, N_average); 
    if (index_azim >= 0) {
        *n_azimuthal = index_azim; 
        status = 1; 
        }     
        //fprintf(stderr,"- sinogram_index: %d     u: %d     v:%d     n_axial: %d     n_azimuthal: %d \n",sinogram_index,*u,*v,*n_axial,*n_azimuthal); 
        //fprintf(stderr,"- sinogram_index: %d    v: %d     %d  %d  %d   |   %d  %d \n",sinogram_index,(j-1-(c-sinogram_index)) + (int)((64-j)/2), c, j, n, (j-1-(c-sinogram_index)), (int)((64-j)/2)); 
    return status; 
}



unsigned int temporary_hack_index_to_bin_span(unsigned int bin_index, unsigned int Span, unsigned int N_segments, int* michelogram_sinogram, int *michelogram_planes, unsigned int N_radial_bins, unsigned int N_angles, unsigned int N_sinograms, unsigned int N_azimuthal, unsigned int *n_axial, unsigned int *n_azimuthal,unsigned int *n_time, unsigned int *u, unsigned int *v)
{
    unsigned int N = N_radial_bins * N_angles; 
//fprintf(stderr,"t 1   N: %d,   N_radial_bins: %d,   N_angles: %d \n", N, N_radial_bins, N_angles); 
    unsigned int sinogram_index = floor(bin_index / N); 
    unsigned int angle_index    = floor((bin_index-sinogram_index*N)/N_radial_bins ); 
    unsigned int radial_index   = bin_index-sinogram_index*N-angle_index*N_radial_bins; 
    unsigned int azimuthal_angle = 0; 
//fprintf(stderr,"t 2\n");
    unsigned int n_rings = 64;  // hack: should be a parameter 
    if (Span == 11)             // hack - mMR and BrainPET
        n_rings = 64; 
    if (Span == 9)
        n_rings = 77; 

    *n_axial = angle_index; 
    *n_time = 0; 
    *u = radial_index;   

    *v = 0;    
    int i=0, c=0, j=0, d=0, n=0;           
    
    int azimuthal_bin = -1; 
    
    if (sinogram_index < n_rings) {
        //fprintf(stderr,"* sinogram_index: %d    v: %d \n",sinogram_index,sinogram_index); 
        d = 0;
        azimuthal_angle = 0;  
        azimuthal_bin   = n_rings-1-sinogram_index; 
    }
    else {
        i=0; c=n_rings-1; j=0; d=0; n=0;
        n = 0;   
        for (i=1; i < 61; i++) {
            n++; 
            j = (n_rings-i); 
            c += j; 
                if (sinogram_index <= c) {
                    d=0;
                    break; 
                }
            c += j; 
                if (sinogram_index <= c) {
                    d=1; 
                    break; 
                }
        }
//fprintf(stderr,"t 3\n");
    azimuthal_angle = n;
    azimuthal_bin = c-sinogram_index; 
    } 
    // Now variable 'azimuthal_angle' contains the azimuthal index, with 0 at the center. Variable d indicates the direction, positive or negative. 
    // 'azimuthal_bin' indicates the azimuthal bin 
//fprintf(stderr,"t 4\n");
    int ring0, ring1, plane;
    int segment;  

    if (d==0) 
        {
        ring0 = azimuthal_bin; 
        ring1 = ring0 + azimuthal_angle; 
        }
    else 
        {
        ring1 = azimuthal_bin; 
        ring0 = ring1 + azimuthal_angle;  
        }
//    if (ring0>63)
//        fprintf(stderr,"ring0 > 63 -  ring0: %d   ring1: %d   azimuthal_bin: %d   azimuthal_angle: %d   direction: %d \n",ring0,ring1,azimuthal_bin,azimuthal_angle,d);
//    if (ring1>63)
//        fprintf(stderr,"ring1 > 63 -  ring0: %d   ring1: %d   azimuthal_bin: %d   azimuthal_angle: %d   direction: %d \n",ring0,ring1,azimuthal_bin,azimuthal_angle,d);
//    if (ring0<0)
//        fprintf(stderr,"ring0 < 0  -  ring0: %d   ring1: %d   azimuthal_bin: %d   azimuthal_angle: %d   direction: %d \n",ring0,ring1,azimuthal_bin,azimuthal_angle,d);
//    if (ring1<0)
//        fprintf(stderr,"ring1 < 0  -  ring0: %d   ring1: %d   azimuthal_bin: %d   azimuthal_angle: %d   direction: %d \n",ring0,ring1,azimuthal_bin,azimuthal_angle,d);
//fprintf(stderr,"t 5   n_rings: %d    ring0: %d    ring1: %d \n",n_rings, ring0, ring1 );
//fprintf(stderr,"t 6   bin_index: %d,  sinogram_index: %d,  angle_index: %d,  radial_index: %d \n",bin_index, sinogram_index, angle_index, radial_index);
    segment = michelogram_sinogram[n_rings*ring1 + ring0]; 

    if (segment==-1)    //The LOR falls outside of the valid segments
        return 0;
    int offset = 0; 
    if (Span==1) {
        offset = (int) floor(abs((segment-(N_segments-1)/2))/2); 
    }
    else {
        offset = Span*abs(segment-(N_segments-1)/2);
        if (segment!= (N_segments-1)/2) 
            offset = offset-(Span-1)/2;  
    } 
//fprintf(stderr,"t 7\n");
//    fprintf(stdout, "*** sinogram: %d    azimuthal_angle: %d    bin: %d    direction: %d    ring0: %d     ring1: %d \n",sinogram_index, azimuthal_angle, azimuthal_bin, d, ring0, ring1);

    plane   = offset + michelogram_planes[n_rings*ring1 + ring0];

//    fprintf(stdout, "    segment: %d    plane: %d    offset: %d \n",segment, plane, offset);

    *v = plane; 
    *n_azimuthal = segment; 

//fprintf(stdout, 'plane: %d   segment: %d    n_axial: %d    u: %d \n', plane, segment, angle_index, radial_index); 
    
    return 1; 
//    return 0;
}



/* (Note: this function is not exported) Compress a static projection*/ 
int compress_projection(unsigned int *projection, StaticProjection *static_projection, unsigned int time_start, unsigned int time_end, unsigned int N_counts, unsigned int N_active_locations)
{
    int status = STATUS_SUCCESS; 
    // Allocate memory for the compressed data: array of locations of the bins and array of counts: 
    float          *counts    = (float *) malloc(sizeof(float) * N_active_locations);  //FIXME: deallocate 
    unsigned short *locations = (unsigned short *) malloc(3 * sizeof(unsigned short) * N_active_locations); //FIXME: deallocate 

    // Walk along the projection bins, find non-zero entries and store corresponding location and number of counts in the 
    // compressed projection data structure: 
    unsigned int bin_ax, bin_az, bin_u, bin_v, index, counter; 
    unsigned int N_axial      = static_projection->layout->binning->N_axial; 
    unsigned int N_azimuthal  = static_projection->layout->binning->N_azimuthal; 
    unsigned int N_u          = static_projection->layout->binning->N_u; 
    unsigned int N_v          = static_projection->layout->binning->N_v; 
    unsigned int *offsets_mat = static_projection->layout->offsets->matrix; 

    //fprintf(stdout,"Events per time bin: %d \n",N_events_time_bin); 
    counter = 0; 
    for (bin_ax=0; bin_ax<N_axial; bin_ax++) {
        for (bin_az=0; bin_az<N_azimuthal; bin_az++) { 
            // new plane: update offsets matrix: 
            offsets_mat[bin_az*N_axial + bin_ax] = counter;  
            for (bin_u=0; bin_u<N_u; bin_u++) {
                for (bin_v=0; bin_v<N_v; bin_v++) {
                    index = bin_az*(N_u*N_v*N_axial) + bin_ax*(N_u*N_v) + bin_u*(N_v) + bin_v; //FIXME: make faster: pre-compute products
                    if (projection[index]) {
                        // append event: add to the array of locations 
                        if (counter <= N_active_locations) {
                            counts[counter] = projection[index]; 
                            locations[counter*3+0] = bin_u; 
                            locations[counter*3+1] = bin_v;
                            locations[counter*3+2] = 0; 
                        }
                        else {
                            fprintf(stderr,"Compression: counter is %d and N_locations is %d.\n",counter,N_active_locations);
                            fprintf(stderr,"Unexpected error: counter > N_active_locations. \n");
                            return STATUS_UNHANDLED_ERROR; 
                        }
                        counter += 1; 
                    }
                }
            }
        }
    }

    fprintf(stderr,"Compression: N_counts is %d and N_locations is %d.\n",N_counts,N_active_locations);
    static_projection->counts = counts; 
    static_projection->locations = locations; 
    static_projection->time_start = time_start; 
    static_projection->time_end = time_end; 
    static_projection->layout->offsets->N_axial = N_axial; 
    static_projection->layout->offsets->N_azimuthal = N_azimuthal; 
    static_projection->N_counts          = N_counts;  
    static_projection->N_locations       = N_active_locations; 
    static_projection->compression_ratio = (float) N_active_locations / (float) (N_u*N_v*N_axial*N_azimuthal);
    static_projection->listmode_loss     = (float) N_counts / (float) N_active_locations;
    return status;
}



/*
Converts petlink32 listmode data to a dynamic projection. 
*/
typedef int (*callback_func_ptr)(int); 
extern int petlink32_to_dynamic_projection_mMR_michelogram(char *filename, int64_t *n_packets, unsigned int *n_radial_bins, unsigned int *n_angles, unsigned int *n_sinograms, unsigned int *n_time_bins, unsigned int *time_bins, unsigned int *n_axial, unsigned int *n_azimuthal, float *angles_axial, float *angles_azimuthal, float *size_u, float *size_v, unsigned int *n_u, unsigned int *n_v, unsigned int *span, unsigned int *n_segments, int *segments_sizes, int* michelogram_sinogram, int* michelogram_planes, callback_func_ptr callback) 
{
    int64_t N_packets  = *n_packets;                        // input parameter: number of packets to read from list-mode data
    
    unsigned int Span = *span; 
    unsigned int N_segments = *n_segments; 

    unsigned int N_radial_bins   = *n_radial_bins;          // input parameter: mMR description 
    unsigned int N_angles        = *n_angles;               // input parameter: mMR description  
    unsigned int N_sinograms     = *n_sinograms;            // input parameter: mMR description 
     
    unsigned int N_axial         = *n_axial;                // input parameter: projection binning 
    unsigned int N_azimuthal     = *n_azimuthal;            // input parameter: projection binning     
   
    float Size_u                 = *size_u;                 // input parameter: projection binning     
    float Size_v                 = *size_v;                 // input parameter: projection binning      
    unsigned int N_u             = *n_u;                    // input parameter: projection binning     
    unsigned int N_v             = *n_v;                    // input parameter: projection binning     
    
    unsigned int N_time_bins     = *n_time_bins;            // input parameter: number of time bins 
    unsigned int *Time_bins      = time_bins;               // input parameter: time bins array; mind that the array has (n_time_bins+1) elements !!
    
    int status = STATUS_SUCCESS;                            // output status 
    unsigned int buffer_size_batch = BUFFER_SIZE_BATCH; 

    fprintf(stderr,"==== petlink32_to_dynamic_projection_mMR ==== \n");
    fprintf(stderr,"N_packets:              %uL \n",N_packets);
    fprintf(stderr,"N_radial_bins:          %d \n",N_radial_bins);
    fprintf(stderr,"N_angles:               %d \n",N_angles);
    fprintf(stderr,"N_sinograms:            %d \n",N_sinograms);
    fprintf(stderr,"N_axial:                %d \n",N_axial);
    fprintf(stderr,"N_azimuthal:            %d \n",N_azimuthal);
    fprintf(stderr,"Angles_axial:           %f .. \n",angles_axial[0]);
    fprintf(stderr,"Angles_azimuthal:       %f .. \n",angles_azimuthal[0]);
    fprintf(stderr,"Size_u:                 %f \n",Size_u);
    fprintf(stderr,"Size_v:                 %f \n",Size_v);
    fprintf(stderr,"N_u:                    %d \n",N_u);
    fprintf(stderr,"N_v:                    %d \n",N_v);
    fprintf(stderr,"N_time_bins:            %d \n",N_time_bins);
    fprintf(stderr,"time_bin[0]:            %d \n",Time_bins[0]);
    fprintf(stderr,"time_bin[%d]:           %d \n",N_time_bins,Time_bins[N_time_bins]);
    fprintf(stderr,"Span:                   %d \n",Span);
    fprintf(stderr,"N_segments:             %d \n",N_segments);
    fprintf(stderr,"============================================ \n");
     
    // Make a buffer to store one batch of packets 
    char batch_buffer[buffer_size_batch*BYTES_PER_PACKET]; 

    // Open the petlink 32-bit listmode file
    FILE *fid; 
    fid=fopen(filename, "rb");
    if (fid == NULL) {
        fprintf(stderr,"Failed to open listmode file. \n");
        status = STATUS_IO_ERROR; 
        return status; 
    }

//    // Instantiate data structures for the dynamic projection     
//    if (globalobject.initialised) {
//        deallocate_globalobject(); 
//    }
    globalobject.dynamic_projection = instantiate_dynamic_projection(N_time_bins);
    globalobject.dynamic_projection_delay = instantiate_dynamic_projection(N_time_bins);
    globalobject.stats              = instantiate_stats(); ; 

    // Define variables for the conversion of the list-mode data into bins 
    int i, j; 
    unsigned int elapsed_time_ms = 0; 
    unsigned int bin_axial, bin_azimuthal, bin_time, bin_u, bin_v, tmp_index; 
    LOR lor; 
    unsigned int *tmp_sinogram = (unsigned int *) malloc(sizeof(unsigned int)*N_axial*N_azimuthal*N_u*N_v); 
    memset((void *) tmp_sinogram, 0, sizeof(unsigned int)*N_axial*N_azimuthal*N_u*N_v );     
    unsigned int *global_sinogram = (unsigned int *) malloc(sizeof(unsigned int)*N_axial*N_azimuthal*N_u*N_v); 
    memset((void *) global_sinogram, 0, sizeof(unsigned int)*N_axial*N_azimuthal*N_u*N_v ); 
    unsigned int *tmp_sinogram_delay = (unsigned int *) malloc(sizeof(unsigned int)*N_axial*N_azimuthal*N_u*N_v); 
    memset((void *) tmp_sinogram_delay, 0, sizeof(unsigned int)*N_axial*N_azimuthal*N_u*N_v );     
    unsigned int *global_sinogram_delay = (unsigned int *) malloc(sizeof(unsigned int)*N_axial*N_azimuthal*N_u*N_v); 
    memset((void *) global_sinogram_delay, 0, sizeof(unsigned int)*N_axial*N_azimuthal*N_u*N_v ); 

    // Read packets, decode them and populate the dynamic projection data structure 
    Packet packet;
    int64_t N_batches = floor(N_packets / buffer_size_batch) + 1; 
    int64_t n_packets_last_batch = N_packets - (N_batches-1)*buffer_size_batch; 
    int64_t n_packets_batch; 

    unsigned int time_bin_index = 0; // time-bin index 
    unsigned int time_start = Time_bins[0];       // Time start for the first time bin 
    unsigned int time_end   = Time_bins[1];       // Time end for the first time bin 
    unsigned int time_total = time_end - time_start;
    unsigned int waiting_for_firt_packet_time = 1;   // flag that indicates if a packet of type time_elapsed has already been received
    unsigned int initial_time = 0;                   // time indicated in the first packet of type time_elapsed 
    unsigned int done = 0;
    unsigned int N_events_time_bin = 0; 
    int64_t N_events_total    = 0;
    unsigned int N_locations_time_bin = 0;       // Number of active interaction locations for one time bin 
    unsigned int N_locations_global   = 0;       // Number of active interaction locations in the global static projection 
    unsigned int N_locations_dynamic  = 0;       // Total number of active locations in the dynamic sequence (sum of N_locations_time_bin)

    unsigned int N_events_time_bin_delay = 0; 
    int64_t N_events_total_delay    = 0;
    unsigned int N_locations_time_bin_delay = 0;
    unsigned int N_locations_global_delay = 0;
    unsigned int N_locations_dynamic_delay = 0;
    
    //fprintf(stderr, "N_batches: %ul    n_packets_batch: %ul  %ul    N_packets: \n",N_batches,n_packets_batch,N_packets);

    unsigned int callback_time=0; 
    int completion_status = 0; 

    for (j = 0; j < N_batches; j++) {
        if(done) {break; }
        if (j==N_batches-1) {
            n_packets_batch = n_packets_last_batch; 
        }
        else {
            n_packets_batch = buffer_size_batch; 
        }
        status = read_packets(fid, batch_buffer, n_packets_batch); 
        if (status!=STATUS_SUCCESS) {
            fprintf(stderr,"Problem reading the listmode data.\n"); 
            //return status; 
            done=1;
            callback(100);
            break; 
        }
        for (i = 0; i < n_packets_batch; i++) { 
            if (done) {break; }
            memcpy ( packet.buffer, batch_buffer+i*BYTES_PER_PACKET, BYTES_PER_PACKET );
            status = decode_packet(&packet);
            if (status!=STATUS_SUCCESS) {
                fprintf(stderr,"Problem decoding the listmode data.\n"); 
                fclose(fid); 
                return status; 
            }    
            if (packet.type == TYPE_EVENT_PROMPT) {
                if (elapsed_time_ms >= time_start) {
                    globalobject.stats->n_prompt = globalobject.stats->n_prompt + 1; 
                    // FIXME: convert to LOR coordinates and then to bins (replace temporary_hack_index_to_bin)
                    unsigned int ok = temporary_hack_index_to_bin_span(packet.bin_index, Span, N_segments, michelogram_sinogram, michelogram_planes, N_radial_bins, N_angles, N_sinograms, N_azimuthal, &bin_axial, &bin_azimuthal,&bin_time, &bin_u, &bin_v);                 
                    //unsigned int ok = temporary_hack_index_to_bin(packet.bin_index, N_radial_bins, N_angles, N_sinograms, N_azimuthal, &bin_axial, &bin_azimuthal,&bin_time, &bin_u, &bin_v);                 
                    // Convert mMR index into LOR end-points coordinates 
//                    index_to_LOR_mMR(packet.bin_index,N_radial_bins,N_angles,N_sinograms,&lor); 
                    // Convert LOR coordinates into binning index 
//                    LOR_to_bin(&lor,&binning,&n_axial,&n_azimuthal,&n_time,&u,&v); 
                    // Histogram the event into the temporary sinogram 
                    if (ok) {
                        tmp_index = bin_azimuthal*(N_u*N_v*N_axial) + bin_axial*(N_u*N_v) + bin_u*(N_v) + bin_v;  
                        if (tmp_sinogram[tmp_index] == 0) {
                            N_locations_time_bin += 1; 
                            N_locations_dynamic  += 1; 
                        }
                        if (global_sinogram[tmp_index] == 0) {
                            N_locations_global += 1; 
                        }
                        tmp_sinogram[tmp_index] += 1; 
                        global_sinogram[tmp_index] += 1; 
                        N_events_time_bin += 1; 
                        N_events_total    += 1;   
                    }
                }
            }
            else if (packet.type == TYPE_EVENT_DELAY) { 
                if (elapsed_time_ms >= time_start) {
                    globalobject.stats->n_delayed = globalobject.stats->n_delayed + 1; 
                
                    unsigned int ok = temporary_hack_index_to_bin_span(packet.bin_index, Span, N_segments, michelogram_sinogram, michelogram_planes, N_radial_bins, N_angles, N_sinograms, N_azimuthal, &bin_axial, &bin_azimuthal,&bin_time, &bin_u, &bin_v);                 
                    // Histogram the event into the temporary sinogram 
                    if (ok) {
                        tmp_index = bin_azimuthal*(N_u*N_v*N_axial) + bin_axial*(N_u*N_v) + bin_u*(N_v) + bin_v;  
                        if (tmp_sinogram_delay[tmp_index] == 0) {
                            N_locations_time_bin_delay += 1; 
                            N_locations_dynamic_delay  += 1; 
                        }
                        if (global_sinogram_delay[tmp_index] == 0) {
                            N_locations_global_delay += 1; 
                        }
                        tmp_sinogram_delay[tmp_index] += 1; 
                        global_sinogram_delay[tmp_index] += 1; 
                        N_events_time_bin_delay += 1; 
                        N_events_total_delay    += 1;   
                    }
                }
            }
            else if (packet.type == TYPE_TIME_ELAPSED) {
                if (waiting_for_firt_packet_time) {
                    initial_time = packet.time_ms; 
                    waiting_for_firt_packet_time = 0; 
                }
                elapsed_time_ms = packet.time_ms - initial_time; 
                // Time packet received: verify if the time frame changes now 
                //fprintf(stderr,"Elapsed time packet: %d [ms] \n",elapsed_time_ms);
                if (elapsed_time_ms >= time_end) {
                    fprintf(stderr,"End of time frame %d - elapsed scanning time: %d [ms], frame end time: %d [ms].\n",time_bin_index,elapsed_time_ms,time_end);
                    if (time_bin_index < N_time_bins) {
                        // New time bin starts here: make compressed projection for previous time bin; 
                        // clear the temporary sinogram and the events-per-frame counters 
                        fprintf(stderr,"Compressing time bin %d:   %d -> %d [ms]   (%d events) \n",time_bin_index, time_start,time_end,N_events_time_bin);    
                        // 1) Instantiate data sctructures for static projection 
                        // - Prompts
                        ProjectionBinning      *binning = instantiate_binning(N_axial,N_azimuthal,angles_axial, angles_azimuthal,Size_u,Size_v,N_u,N_v);         
                        OffsetsMatrix          *offsets = instantiate_offsets(N_axial,N_azimuthal);  //note: memory alloc in this function
                        StaticProjectionLayout *layout  = instantiate_layout(offsets,binning); 
                        StaticProjection       *static_projection = instantiate_static_projection(time_start,time_end,0,0,0,0,NULL,NULL,layout); 
                        globalobject.dynamic_projection->static_projections[time_bin_index] = static_projection; 
                        // - Delays
                        ProjectionBinning      *binning_delay = instantiate_binning(N_axial,N_azimuthal,angles_axial, angles_azimuthal,Size_u,Size_v,N_u,N_v);         
                        OffsetsMatrix          *offsets_delay = instantiate_offsets(N_axial,N_azimuthal);  //note: memory alloc in this function
                        StaticProjectionLayout *layout_delay  = instantiate_layout(offsets_delay,binning_delay); 
                        StaticProjection       *static_projection_delay = instantiate_static_projection(time_start,time_end,0,0,0,0,NULL,NULL,layout_delay); 
                        globalobject.dynamic_projection_delay->static_projections[time_bin_index] = static_projection_delay; 
                        // 2) Compress 
                        status = compress_projection(tmp_sinogram, globalobject.dynamic_projection->static_projections[time_bin_index], time_start, time_end, N_events_time_bin, N_locations_time_bin); 
                        if (status!=STATUS_SUCCESS) 
                            return status; 
                        status = compress_projection(tmp_sinogram_delay, globalobject.dynamic_projection_delay->static_projections[time_bin_index], time_start, time_end, N_events_time_bin_delay, N_locations_time_bin_delay); 
                        if (status!=STATUS_SUCCESS) 
                            return status;  
                        // 3) Clear temporary sinogram 
                        memset((void *) tmp_sinogram, 0, sizeof(unsigned int)*N_axial*N_azimuthal*N_u*N_v ); 
                        memset((void *) tmp_sinogram_delay, 0, sizeof(unsigned int)*N_axial*N_azimuthal*N_u*N_v ); 
                        // 4) Update current time bin information and reset counters 
                        // - Prompts
                        N_events_time_bin = 0; 
                        N_locations_time_bin = 0; 
                        // - Delays
                        N_events_time_bin_delay = 0; 
                        N_locations_time_bin_delay = 0; 
                        time_bin_index += 1;
                        if (time_bin_index < N_time_bins){
                            time_start = Time_bins[time_bin_index];
                            time_end   = Time_bins[time_bin_index+1];
                        }
                    }
                    else {
                        fprintf(stderr,"2- Stopped processing packets because time_bin_index == N_time_bins. \n");
                        done = 1; 
                        callback(100);
                        break; 
                    }
                }
                // Stop processing packets if the time flag from the list-mode data stream is larger than the largest time bin end. 
                if (elapsed_time_ms >= Time_bins[N_time_bins]) {
//                    fprintf(stdout,"1- Stopped processing packets because the time flag from the list-mode data stream is larger than the largest time bin end (%d >= %d)\n",elapsed_time_ms,Time_bins[N_time_bins]);
                    done = 1;
                    callback(100);
                    break;
                }     
                // status callback function
                callback_time += 1; 
                if (callback_time >= time_total/100) {
                    completion_status += 1;
                    callback_time = 0; 
                    callback(completion_status);    
                }           
                globalobject.stats->n_elapsedtime = globalobject.stats->n_elapsedtime + 1; } 
            else if (packet.type == TYPE_TIME_DEADTIME) {
                globalobject.stats->n_deadtime = globalobject.stats->n_deadtime + 1; } 
            else if (packet.tag == TAG_MOTION) {     // notice: just check the tag, no specific type for motion (this could be extended)
                globalobject.stats->n_motion = globalobject.stats->n_motion + 1; }         
            else if (packet.type == TYPE_MONITORING_GATING0) {
                globalobject.stats->n_gating0 = globalobject.stats->n_gating0 + 1; } 
            else if (packet.type == TYPE_MONITORING_GATING1) {
                globalobject.stats->n_gating1 = globalobject.stats->n_gating1 + 1; } 
            else if (packet.type == TYPE_MONITORING_GATING2) {
                globalobject.stats->n_gating2 = globalobject.stats->n_gating2 + 1; } 
            else if (packet.type == TYPE_MONITORING_GATING3) {
                globalobject.stats->n_gating3 = globalobject.stats->n_gating3 + 1; }                 
            else if (packet.type == TYPE_MONITORING_GATING4) {
                globalobject.stats->n_gating4 = globalobject.stats->n_gating4 + 1; } 
            else if (packet.type == TYPE_MONITORING_GATING5) {
                globalobject.stats->n_gating5 = globalobject.stats->n_gating5 + 1; } 
            else if (packet.type == TYPE_MONITORING_GATING6) {
                globalobject.stats->n_gating6 = globalobject.stats->n_gating6 + 1; } 
            else if (packet.type == TYPE_MONITORING_GATING7) {
                globalobject.stats->n_gating7 = globalobject.stats->n_gating7 + 1; }                 
            else if (packet.type == TYPE_MONITORING_MOTION_TRACKING) {
                globalobject.stats->n_tracking = globalobject.stats->n_tracking + 1; } 
            else if (packet.type == TYPE_CONTROL_PET) {
                globalobject.stats->n_control_PET = globalobject.stats->n_control_PET + 1; } 
            else if (packet.type == TYPE_CONTROL_MR) {
                globalobject.stats->n_control_MR = globalobject.stats->n_control_MR + 1; } 
            else if (packet.type == TYPE_CONTROL_OTHER) {
                globalobject.stats->n_control_OTHER = globalobject.stats->n_control_OTHER + 1; } 
            else {
                globalobject.stats->n_unknown = globalobject.stats->n_unknown + 1; } 
        } 
    }
    // If the packets stream has terminated before compressing the last bin, compress it: 
    if (done==0) {
        time_end = elapsed_time_ms; 
        fprintf(stdout,"Compressing time bin %d:   %d -> %d [ms]   (%d events, %d locations) \n", time_bin_index, time_start, time_end, N_events_time_bin, N_locations_time_bin); 
        // 1) Instantiate data sctructures for static projection 
        // - Prompts
        ProjectionBinning      *binning = instantiate_binning(N_axial,N_azimuthal,angles_axial,angles_azimuthal,Size_u,Size_v,N_u,N_v);         
        OffsetsMatrix          *offsets = instantiate_offsets(N_axial,N_azimuthal);  //note: memory alloc in this function
        StaticProjectionLayout *layout  = instantiate_layout(offsets,binning); 
        StaticProjection       *static_projection = instantiate_static_projection(time_start,time_end,0,0,0,0,NULL,NULL,layout); 
        globalobject.dynamic_projection->static_projections[time_bin_index] = static_projection; 
        // - Delays 
        ProjectionBinning      *binning_delay = instantiate_binning(N_axial,N_azimuthal,angles_axial,angles_azimuthal,Size_u,Size_v,N_u,N_v);         
        OffsetsMatrix          *offsets_delay = instantiate_offsets(N_axial,N_azimuthal);  //note: memory alloc in this function
        StaticProjectionLayout *layout_delay  = instantiate_layout(offsets_delay,binning_delay); 
        StaticProjection       *static_projection_delay = instantiate_static_projection(time_start,time_end,0,0,0,0,NULL,NULL,layout_delay); 
        globalobject.dynamic_projection_delay->static_projections[time_bin_index] = static_projection_delay; 
        // 2) Compress 
        status = compress_projection(tmp_sinogram, globalobject.dynamic_projection->static_projections[time_bin_index], time_start, time_end, N_events_time_bin, N_locations_time_bin);
        if (status!=STATUS_SUCCESS) 
            return status; 
        status = compress_projection(tmp_sinogram_delay, globalobject.dynamic_projection_delay->static_projections[time_bin_index], time_start, time_end, N_events_time_bin_delay, N_locations_time_bin_delay);
        if (status!=STATUS_SUCCESS) 
            return status; 
        time_bin_index += 1;        
    }

    // Compress the global sinogram 
    // - Prompts
    if (N_time_bins == 1){
        fprintf(stderr,"Compressing the global static projection:   %d -> %d [ms]   (%lld events, %d locations) \n",time_start, time_end, N_events_total,N_locations_global);
        // - Prompts
        ProjectionBinning      *global_binning = instantiate_binning(N_axial,N_azimuthal,angles_axial,angles_azimuthal,Size_u,Size_v,N_u,N_v);
        OffsetsMatrix          *global_offsets = instantiate_offsets(N_axial,N_azimuthal);
        StaticProjectionLayout *global_layout  = instantiate_layout(global_offsets,global_binning);
        StaticProjection       *global_static_projection = instantiate_static_projection(time_start, time_end,0,0,0,0,NULL,NULL,global_layout);
        globalobject.dynamic_projection->global_static_projection = global_static_projection;
        status = compress_projection(global_sinogram, globalobject.dynamic_projection->global_static_projection, time_start, time_end, N_events_total, N_locations_global);
        if (status!=STATUS_SUCCESS)
            return status;
        // - Delays
        ProjectionBinning      *global_binning_delay = instantiate_binning(N_axial,N_azimuthal,angles_axial,angles_azimuthal,Size_u,Size_v,N_u,N_v);
        OffsetsMatrix          *global_offsets_delay = instantiate_offsets(N_axial,N_azimuthal);
        StaticProjectionLayout *global_layout_delay  = instantiate_layout(global_offsets_delay,global_binning_delay);
        StaticProjection       *global_static_projection_delay = instantiate_static_projection(time_start, time_end,0,0,0,0,NULL,NULL,global_layout_delay);
        globalobject.dynamic_projection_delay->global_static_projection = global_static_projection_delay;
        status = compress_projection(global_sinogram_delay, globalobject.dynamic_projection_delay->global_static_projection, time_start, time_end, N_events_total_delay, N_locations_global_delay);
        if (status!=STATUS_SUCCESS)
            return status;
    }
    else{
        fprintf(stderr,"Compressing the global static projection:   %d -> %d [ms]   (%lld events, %d locations) \n",0,elapsed_time_ms,N_events_total,N_locations_global);
        // - Prompts
        ProjectionBinning      *global_binning = instantiate_binning(N_axial,N_azimuthal,angles_axial,angles_azimuthal,Size_u,Size_v,N_u,N_v);
        OffsetsMatrix          *global_offsets = instantiate_offsets(N_axial,N_azimuthal);
        StaticProjectionLayout *global_layout  = instantiate_layout(global_offsets,global_binning);
        StaticProjection       *global_static_projection = instantiate_static_projection(0,elapsed_time_ms,0,0,0,0,NULL,NULL,global_layout);
        globalobject.dynamic_projection->global_static_projection = global_static_projection;
        status = compress_projection(global_sinogram, globalobject.dynamic_projection->global_static_projection, 0, elapsed_time_ms, N_events_total, N_locations_global);
        if (status!=STATUS_SUCCESS)
            return status;
        // - Delays
        ProjectionBinning      *global_binning_delay = instantiate_binning(N_axial,N_azimuthal,angles_axial,angles_azimuthal,Size_u,Size_v,N_u,N_v);
        OffsetsMatrix          *global_offsets_delay = instantiate_offsets(N_axial,N_azimuthal);
        StaticProjectionLayout *global_layout_delay  = instantiate_layout(global_offsets_delay,global_binning_delay);
        StaticProjection       *global_static_projection_delay = instantiate_static_projection(0,elapsed_time_ms,0,0,0,0,NULL,NULL,global_layout_delay);
        globalobject.dynamic_projection_delay->global_static_projection = global_static_projection_delay;
        status = compress_projection(global_sinogram_delay, globalobject.dynamic_projection_delay->global_static_projection, 0, elapsed_time_ms, N_events_total_delay, N_locations_global_delay);
        if (status!=STATUS_SUCCESS)
            return status;
    }
    // Close file 
    fclose(fid); 

    // Release allocated memory 
    free((void*) tmp_sinogram); 
    free((void*) global_sinogram); 
    free((void*) tmp_sinogram_delay); 
    free((void*) global_sinogram_delay);    
    // FIXME: destroy the data structures, except for the global data 

    globalobject.dynamic_projection->N_time_bins       = time_bin_index; 
    globalobject.dynamic_projection->N_counts          = N_events_total;
    globalobject.dynamic_projection->N_locations       = N_locations_dynamic; 
    globalobject.dynamic_projection->compression_ratio = (float) N_locations_global/ (float) (N_u*N_v*N_axial*N_azimuthal*N_time_bins); 
    globalobject.dynamic_projection->dynamic_inflation = (float) N_locations_dynamic/ (float) N_locations_global; 
    globalobject.dynamic_projection->listmode_loss     = (float) N_events_total/ (float) N_locations_dynamic; 
    globalobject.dynamic_projection->time_start        = time_start; 
    globalobject.dynamic_projection->time_end          = elapsed_time_ms;     

    globalobject.dynamic_projection_delay->N_time_bins       = time_bin_index; 
    globalobject.dynamic_projection_delay->N_counts          = N_events_total_delay;
    globalobject.dynamic_projection_delay->N_locations       = N_locations_dynamic_delay; 
    globalobject.dynamic_projection_delay->compression_ratio = (float) N_locations_global_delay/ (float) (N_u*N_v*N_axial*N_azimuthal*N_time_bins); 
    globalobject.dynamic_projection_delay->dynamic_inflation = (float) N_locations_dynamic_delay/ (float) N_locations_global_delay; 
    globalobject.dynamic_projection_delay->listmode_loss     = (float) N_events_total_delay/ (float) N_locations_dynamic_delay; 
    globalobject.dynamic_projection_delay->time_start        = time_start; 
    globalobject.dynamic_projection_delay->time_end          = elapsed_time_ms;     

    globalobject.has_projection = 1; 
    return status; 
}




extern int petlink32_to_dynamic_projection_cyclic_mMR_michelogram(char *filename, int64_t *n_packets, unsigned int *n_radial_bins, unsigned int *n_angles, unsigned int *n_sinograms, unsigned int *n_frames, unsigned int *n_repetitions, unsigned int *time_bins, unsigned int *n_axial, unsigned int *n_azimuthal, float *angles_axial, float *angles_azimuthal, float *size_u, float *size_v, unsigned int *n_u, unsigned int *n_v, unsigned int *span, unsigned int *n_segments, int *segments_sizes, int* michelogram_sinogram, int* michelogram_planes, callback_func_ptr callback) 
{
    int64_t N_packets  = *n_packets;                        // input parameter: number of packets to read from list-mode data
    
    unsigned int Span = *span; 
    unsigned int N_segments = *n_segments; 

    unsigned int N_radial_bins   = *n_radial_bins;          // input parameter: mMR description 
    unsigned int N_angles        = *n_angles;               // input parameter: mMR description  
    unsigned int N_sinograms     = *n_sinograms;            // input parameter: mMR description 
     
    unsigned int N_axial         = *n_axial;                // input parameter: projection binning 
    unsigned int N_azimuthal     = *n_azimuthal;            // input parameter: projection binning     
   
    float Size_u                 = *size_u;                 // input parameter: projection binning     
    float Size_v                 = *size_v;                 // input parameter: projection binning      
    unsigned int N_u             = *n_u;                    // input parameter: projection binning     
    unsigned int N_v             = *n_v;                    // input parameter: projection binning     
    
    unsigned int N_frames        = *n_frames;               // input parameter
    unsigned int N_repetitions   = *n_repetitions;          // input parameter
    
    int status = STATUS_SUCCESS;                            // output status 

    fprintf(stderr,"==== petlink32_to_dynamic_projection_cyclic_mMR ==== \n");
    fprintf(stderr,"N_radial_bins:          %d \n",N_radial_bins);
    fprintf(stderr,"N_angles:               %d \n",N_angles);
    fprintf(stderr,"N_sinograms:            %d \n",N_sinograms);
    fprintf(stderr,"N_axial:                %d \n",N_axial);
    fprintf(stderr,"N_azimuthal:            %d \n",N_azimuthal);
    fprintf(stderr,"Angles_axial:           %f .. \n",angles_axial[0]);
    fprintf(stderr,"Angles_azimuthal:       %f .. \n",angles_azimuthal[0]);
    fprintf(stderr,"Size_u:                 %f \n",Size_u);
    fprintf(stderr,"Size_v:                 %f \n",Size_v);
    fprintf(stderr,"N_u:                    %d \n",N_u);
    fprintf(stderr,"N_v:                    %d \n",N_v);
    fprintf(stderr,"N_frames   :            %d \n",N_frames);
    fprintf(stderr,"N_repetitions   :       %d \n",N_repetitions);
    fprintf(stderr,"Span:                   %d \n",Span);
    fprintf(stderr,"N_segments:             %d \n",N_segments);
    fprintf(stderr,"============================================ \n");
     
    // Make a buffer to store one batch of packets 
    unsigned int buffer_size_batch = BUFFER_SIZE_BATCH; 
    char batch_buffer[buffer_size_batch*BYTES_PER_PACKET]; 

    // Open the petlink 32-bit listmode file
    FILE *fid; 
    fid=fopen(filename, "rb");
    if (fid == NULL) {
        fprintf(stderr,"Failed to open listmode file. \n");
        status = STATUS_IO_ERROR; 
        return status; 
    }
//fprintf(stderr,"xxx 1\n");
//    // Instantiate data structures for the dynamic projection     
//    if (globalobject.initialised) {
//        deallocate_globalobject(); 
//    }
    globalobject.dynamic_projection = instantiate_dynamic_projection(N_frames+1);
    globalobject.dynamic_projection_delay = instantiate_dynamic_projection(N_frames+1);
    globalobject.stats              = instantiate_stats(); ; 

    // Define variables for the conversion of the list-mode data into bins 
    int i, k, h; 
    int64_t j;
    unsigned int elapsed_time_ms = 0; 
    unsigned int bin_axial, bin_azimuthal, bin_time, bin_u, bin_v, tmp_index; 
    LOR lor; 
    
    // Instantiate N_frames temporary sinograms: 
    unsigned int *tmp_sinogram;
    unsigned int *tmp_sinogram_delay;
    unsigned int *tmp_sinograms[MAX_FRAMES_CYCLIC]; 
    unsigned int *tmp_sinograms_delay[MAX_FRAMES_CYCLIC]; 
    
    for (i=0; i<N_frames; i++) {
        tmp_sinogram = (unsigned int *) malloc(sizeof(unsigned int)*N_axial*N_azimuthal*N_u*N_v); 
        memset((void *) tmp_sinogram, 0, sizeof(unsigned int)*N_axial*N_azimuthal*N_u*N_v ); 
        tmp_sinograms[i] =  tmp_sinogram; 

        tmp_sinogram_delay = (unsigned int *) malloc(sizeof(unsigned int)*N_axial*N_azimuthal*N_u*N_v); 
        memset((void *) tmp_sinogram_delay, 0, sizeof(unsigned int)*N_axial*N_azimuthal*N_u*N_v );   
        tmp_sinograms_delay[i] =  tmp_sinogram_delay;   
    }

    unsigned int *tmp_sinogram_discarded = (unsigned int *) malloc(sizeof(unsigned int)*N_axial*N_azimuthal*N_u*N_v); 
    memset((void *) tmp_sinogram_discarded, 0, sizeof(unsigned int)*N_axial*N_azimuthal*N_u*N_v ); 
    unsigned int *tmp_sinogram_delay_discarded = (unsigned int *) malloc(sizeof(unsigned int)*N_axial*N_azimuthal*N_u*N_v); 
    memset((void *) tmp_sinogram_delay_discarded, 0, sizeof(unsigned int)*N_axial*N_azimuthal*N_u*N_v ); 

    unsigned int *global_sinogram = (unsigned int *) malloc(sizeof(unsigned int)*N_axial*N_azimuthal*N_u*N_v); 
    memset((void *) global_sinogram, 0, sizeof(unsigned int)*N_axial*N_azimuthal*N_u*N_v ); 
    unsigned int *global_sinogram_delay = (unsigned int *) malloc(sizeof(unsigned int)*N_axial*N_azimuthal*N_u*N_v); 
    memset((void *) global_sinogram_delay, 0, sizeof(unsigned int)*N_axial*N_azimuthal*N_u*N_v ); 

    // Read packets, decode them and populate the dynamic projection data structure 
    Packet packet;
    int64_t N_batches = (int64_t) floor((int64_t) N_packets / (int64_t) buffer_size_batch) + (int64_t)1; 
    int n_packets_last_batch = N_packets - (N_batches-1)*buffer_size_batch; 
    int n_packets_batch; 

    unsigned int N_locations_time_bin[MAX_FRAMES_CYCLIC];
    unsigned int N_events_time_bin[MAX_FRAMES_CYCLIC];
    unsigned int N_locations_time_bin_delay[MAX_FRAMES_CYCLIC];
    unsigned int N_events_time_bin_delay[MAX_FRAMES_CYCLIC];
    memset((void *) N_locations_time_bin, 0, sizeof(unsigned int)*MAX_FRAMES_CYCLIC);
    memset((void *) N_events_time_bin, 0, sizeof(unsigned int)*MAX_FRAMES_CYCLIC);
    memset((void *) N_locations_time_bin_delay, 0, sizeof(unsigned int)*MAX_FRAMES_CYCLIC);
    memset((void *) N_events_time_bin_delay, 0, sizeof(unsigned int)*MAX_FRAMES_CYCLIC);    

    unsigned int time_start;
    unsigned int time_end; 
    unsigned int time_total;
    unsigned int t_start; 
    unsigned int t_end; 

    // find largest and smallest times specified in time_bins. Consider only non-empty frames (i.e. frames with time_start != time_end)
    time_start = 0;
    time_end = 0; 
    unsigned int found_non_empty=0; 
    for (k=0; k<N_repetitions; k++) { 
        for (h=0; h<N_frames; h++) { 
            t_start = time_bins[h*N_repetitions*2+k*2]; 
            t_end   = time_bins[h*N_repetitions*2+k*2+1]; 
            if (t_start!=t_end) {
                if (!found_non_empty) {
                    time_start = t_start; 
                    time_end   = t_end; 
                    found_non_empty = 1; 
                }
                else {
                    if (t_start<time_start)
                        time_start = t_start; 
                    if (t_end>time_end)
                        time_end = t_end; 
                }
            }
        }
    }
    time_total = time_end-time_start; 

    fprintf(stderr, "time_start: %d    time_end: %d \n", time_start, time_end); 
    for (k=0; k<N_repetitions; k++) { 
        for (h=0; h<N_frames; h++) { 
            t_start = time_bins[h*N_repetitions*2+k*2]; 
            t_end   = time_bins[h*N_repetitions*2+k*2+1];  
//            fprintf(stderr,"repetition %d   frame %d:   start: %d  end: %d \n",k,h,t_start,t_end);
        }
    }

    unsigned int waiting_for_firt_packet_time = 1;   // flag that indicates if a packet of type time_elapsed has already been received
    unsigned int initial_time = 0;                   // time indicated in the first packet of type time_elapsed 
    unsigned int done = 0;
    
    int64_t N_events_total    = 0;
    unsigned int N_locations_global   = 0;       // Number of active interaction locations in the global static projection 
    unsigned int N_locations_dynamic  = 0;       // Total number of active locations in the dynamic sequence (sum of N_locations_time_bin)

    int64_t N_events_total_delay    = 0;
    unsigned int N_locations_global_delay = 0;
    unsigned int N_locations_dynamic_delay = 0;

    unsigned int callback_time=0; 
    int completion_status = 0; 

    unsigned int current_frame = N_frames;  
    unsigned int frame_found = 0; 
    tmp_sinogram = tmp_sinogram_discarded; 
    tmp_sinogram_delay = tmp_sinogram_delay_discarded;

    for (j = 0; j < N_batches; j++) {
        if(done) {break; }
        if (j==N_batches-1) {
            n_packets_batch = n_packets_last_batch; 
        }
        else {
            n_packets_batch = buffer_size_batch; 
        }
        status = read_packets(fid, batch_buffer, n_packets_batch); 
        if (status!=STATUS_SUCCESS) {
            fprintf(stderr,"Problem reading the listmode data.\n"); 
            fclose(fid); 
            //return status; 
            status = STATUS_SUCCESS; 
            done = 1;
            callback(100); 
            break; 
        } 
        for (i = 0; i < n_packets_batch; i++) { 
            if (done) {break; }
            memcpy ( packet.buffer, batch_buffer+i*BYTES_PER_PACKET, BYTES_PER_PACKET );
            status = decode_packet(&packet);
            if (status!=STATUS_SUCCESS) {
                fprintf(stderr,"Problem decoding the listmode data.\n"); 
                fclose(fid); 
                return status; 
            } 
               
            if (packet.type == TYPE_EVENT_PROMPT) {
                if (elapsed_time_ms >= time_start) {
                    globalobject.stats->n_prompt = globalobject.stats->n_prompt + 1; 
                    // FIXME: convert to LOR coordinates and then to bins (replace temporary_hack_index_to_bin)
                    unsigned int ok = temporary_hack_index_to_bin_span(packet.bin_index, Span, N_segments, michelogram_sinogram, michelogram_planes, N_radial_bins, N_angles, N_sinograms, N_azimuthal, &bin_axial, &bin_azimuthal,&bin_time, &bin_u, &bin_v);                 
    
                    // Histogram the event into the current sinogram 
                    if (ok) {
                        tmp_index = bin_azimuthal*(N_u*N_v*N_axial) + bin_axial*(N_u*N_v) + bin_u*(N_v) + bin_v;  
                        if (tmp_sinogram[tmp_index] == 0) {
                            N_locations_time_bin[current_frame] += 1; 
                            N_locations_dynamic  += 1; 
                        }
                        if (global_sinogram[tmp_index] == 0) {
                            N_locations_global += 1; 
                        }
                        tmp_sinogram[tmp_index] += 1; 
                        global_sinogram[tmp_index] += 1; 
                        N_events_time_bin[current_frame] += 1; 
                        N_events_total    += 1;   
                    }
                }
            }
            else if (packet.type == TYPE_EVENT_DELAY) {
                if (elapsed_time_ms >= time_start) {
                    globalobject.stats->n_delayed = globalobject.stats->n_delayed + 1; 
                
                    unsigned int ok = temporary_hack_index_to_bin_span(packet.bin_index, Span, N_segments, michelogram_sinogram, michelogram_planes, N_radial_bins, N_angles, N_sinograms, N_azimuthal, &bin_axial, &bin_azimuthal,&bin_time, &bin_u, &bin_v);                 
                    // Histogram the event into the temporary sinogram 
                    if (ok) {
                        tmp_index = bin_azimuthal*(N_u*N_v*N_axial) + bin_axial*(N_u*N_v) + bin_u*(N_v) + bin_v;  
                        if (tmp_sinogram_delay[tmp_index] == 0) {
                            N_locations_time_bin_delay[current_frame] += 1; 
                            N_locations_dynamic_delay  += 1; 
                        }
                        if (global_sinogram_delay[tmp_index] == 0) {
                            N_locations_global_delay += 1; 
                        }
                        tmp_sinogram_delay[tmp_index] += 1; 
                        global_sinogram_delay[tmp_index] += 1; 
                        N_events_time_bin_delay[current_frame] += 1; 
                        N_events_total_delay    += 1;   
                    }
                }
            }
            else if (packet.type == TYPE_TIME_ELAPSED) {
                if (waiting_for_firt_packet_time) {
                    initial_time = packet.time_ms; 
                    waiting_for_firt_packet_time = 0; 
                }
                elapsed_time_ms = packet.time_ms - initial_time; 
                globalobject.stats->n_elapsedtime = globalobject.stats->n_elapsedtime + 1; 

                // Check in which time frame we are now  
                if (elapsed_time_ms >= time_start) {
                    frame_found = 0; 
                    for (k=0; k<N_repetitions; k++) { 
                        for (h=0; h<N_frames; h++) { 
                            t_start = time_bins[h*N_repetitions*2+k*2]; 
                            t_end   = time_bins[h*N_repetitions*2+k*2+1]; 
                            if ( elapsed_time_ms>=t_start && elapsed_time_ms<t_end ) {
                                current_frame = h; 
                                tmp_sinogram = tmp_sinograms[h]; 
                                tmp_sinogram_delay = tmp_sinograms_delay[h]; 
                                frame_found = 1; 
                                break; 
                            }
                        }
                    }
                    if (!frame_found) {
                        current_frame = N_frames;   //last, additional, frame contains discarded counts within start_time and end_time
                        tmp_sinogram = tmp_sinogram_discarded; 
                        tmp_sinogram_delay = tmp_sinogram_delay_discarded; 
                        //fprintf(stderr,"repetition %d   frame %d:   start: %d    end: %d    elapsed: %d     current_frame: %d \n", k, h, t_start, t_end, elapsed_time_ms, current_frame);
                    }
                }

                // Check if it is time to quit 
                if (elapsed_time_ms >= time_end) {
                     fprintf(stderr, "Done:  elapsed_time_ms: %d    time_end: %d \n", elapsed_time_ms, time_end); 
                     done = 1; 
                     callback(100); 
                     break; 
                }

                // Status callback function
                callback_time += 1; 
                if (callback_time >= time_total/100) {
                    completion_status += 1;
                    callback_time = 0; 
                    callback(completion_status);    
                } 
            }

            else if (packet.type == TYPE_TIME_DEADTIME) {
                globalobject.stats->n_deadtime = globalobject.stats->n_deadtime + 1; } 
            else if (packet.tag == TAG_MOTION) {     // notice: just check the tag, no specific type for motion (this could be extended)
                globalobject.stats->n_motion = globalobject.stats->n_motion + 1; }         
            else if (packet.type == TYPE_MONITORING_GATING0) {
                globalobject.stats->n_gating0 = globalobject.stats->n_gating0 + 1; } 
            else if (packet.type == TYPE_MONITORING_GATING1) {
                globalobject.stats->n_gating1 = globalobject.stats->n_gating1 + 1; } 
            else if (packet.type == TYPE_MONITORING_GATING2) {
                globalobject.stats->n_gating2 = globalobject.stats->n_gating2 + 1; } 
            else if (packet.type == TYPE_MONITORING_GATING3) {
                globalobject.stats->n_gating3 = globalobject.stats->n_gating3 + 1; }                 
            else if (packet.type == TYPE_MONITORING_GATING4) {
                globalobject.stats->n_gating4 = globalobject.stats->n_gating4 + 1; } 
            else if (packet.type == TYPE_MONITORING_GATING5) {
                globalobject.stats->n_gating5 = globalobject.stats->n_gating5 + 1; } 
            else if (packet.type == TYPE_MONITORING_GATING6) {
                globalobject.stats->n_gating6 = globalobject.stats->n_gating6 + 1; } 
            else if (packet.type == TYPE_MONITORING_GATING7) {
                globalobject.stats->n_gating7 = globalobject.stats->n_gating7 + 1; }                 
            else if (packet.type == TYPE_MONITORING_MOTION_TRACKING) {
                globalobject.stats->n_tracking = globalobject.stats->n_tracking + 1; } 
            else if (packet.type == TYPE_CONTROL_PET) {
                globalobject.stats->n_control_PET = globalobject.stats->n_control_PET + 1; } 
            else if (packet.type == TYPE_CONTROL_MR) {
                globalobject.stats->n_control_MR = globalobject.stats->n_control_MR + 1; } 
            else if (packet.type == TYPE_CONTROL_OTHER) {
                globalobject.stats->n_control_OTHER = globalobject.stats->n_control_OTHER + 1; } 
            else {
                globalobject.stats->n_unknown = globalobject.stats->n_unknown + 1; } 
        } 
    }
    
    // Compress sinogram frames 
    for (k=0; k <=  N_frames; k++) {   // >= N_frames includes the additional sinogram for discarded counts 
            t_start = 0;  
            t_end   = 0;  // FIXME: compute average
            fprintf(stderr,"Compressing time frame %d:    time_start_ms: %d,  time_end_ms: %d,   events: %d,    locations: %d \n",k, t_start, t_end, N_events_time_bin[k], N_locations_time_bin[k]);    
            // 1) Instantiate data sctructures for static projection 
            // - Prompts 
            ProjectionBinning      *binning = instantiate_binning(N_axial,N_azimuthal,angles_axial, angles_azimuthal,Size_u,Size_v,N_u,N_v);         
            OffsetsMatrix          *offsets = instantiate_offsets(N_axial,N_azimuthal);  //note: memory alloc in this function
            StaticProjectionLayout *layout  = instantiate_layout(offsets,binning); 
            StaticProjection       *static_projection = instantiate_static_projection(t_start,t_end,0,0,0,0,NULL,NULL,layout); 
            globalobject.dynamic_projection->static_projections[k] = static_projection; 
            // - Delays
            ProjectionBinning      *binning_delay = instantiate_binning(N_axial,N_azimuthal,angles_axial, angles_azimuthal,Size_u,Size_v,N_u,N_v);         
            OffsetsMatrix          *offsets_delay = instantiate_offsets(N_axial,N_azimuthal);  //note: memory alloc in this function
            StaticProjectionLayout *layout_delay  = instantiate_layout(offsets_delay,binning_delay); 
            StaticProjection       *static_projection_delay = instantiate_static_projection(t_start,t_end,0,0,0,0,NULL,NULL,layout_delay); 
            globalobject.dynamic_projection_delay->static_projections[k] = static_projection_delay; 
            // 2) Compress 
            if (k==N_frames) {
                tmp_sinogram = tmp_sinogram_discarded; 
                tmp_sinogram_delay = tmp_sinogram_delay_discarded; 
            }
            else {
                tmp_sinogram = tmp_sinograms[k]; 
                tmp_sinogram_delay = tmp_sinograms_delay[k];    
            }
            status = compress_projection(tmp_sinogram, globalobject.dynamic_projection->static_projections[k], t_start, t_end, N_events_time_bin[k], N_locations_time_bin[k]); 
            if (status!=STATUS_SUCCESS) 
                return status; 
            //status = compress_projection(tmp_sinogram_delay, globalobject.dynamic_projection_delay->static_projections[k], t_start, t_end, N_events_time_bin_delay[k], N_locations_time_bin_delay[k]); 
            //if (status!=STATUS_SUCCESS) 
            //    return status;  
    }

    // Compress the global sinogram 
    // - Prompts
    fprintf(stderr,"Compressing the global static projection:   %d -> %d [ms]   (%lld events, %d locations) \n",0,elapsed_time_ms,N_events_total,N_locations_global);    
    ProjectionBinning      *global_binning = instantiate_binning(N_axial,N_azimuthal,angles_axial,angles_azimuthal,Size_u,Size_v,N_u,N_v);         
    OffsetsMatrix          *global_offsets = instantiate_offsets(N_axial,N_azimuthal);  
    StaticProjectionLayout *global_layout  = instantiate_layout(global_offsets,global_binning); 
    StaticProjection       *global_static_projection = instantiate_static_projection(0,elapsed_time_ms,0,0,0,0,NULL,NULL,global_layout); 
    globalobject.dynamic_projection->global_static_projection = global_static_projection; 
    status = compress_projection(global_sinogram, globalobject.dynamic_projection->global_static_projection, 0, elapsed_time_ms, N_events_total, N_locations_global);  
    if (status!=STATUS_SUCCESS) 
        return status; 
    // - Delays
    ProjectionBinning      *global_binning_delay = instantiate_binning(N_axial,N_azimuthal,angles_axial,angles_azimuthal,Size_u,Size_v,N_u,N_v);         
    OffsetsMatrix          *global_offsets_delay = instantiate_offsets(N_axial,N_azimuthal);  
    StaticProjectionLayout *global_layout_delay  = instantiate_layout(global_offsets_delay,global_binning_delay); 
    StaticProjection       *global_static_projection_delay = instantiate_static_projection(0,elapsed_time_ms,0,0,0,0,NULL,NULL,global_layout_delay); 
    globalobject.dynamic_projection_delay->global_static_projection = global_static_projection_delay; 
    status = compress_projection(global_sinogram_delay, globalobject.dynamic_projection_delay->global_static_projection, 0, elapsed_time_ms, N_events_total_delay, N_locations_global_delay);  
    if (status!=STATUS_SUCCESS) 
        return status; 

    // Close file 
//    fclose(fid); 

    // Release allocated memory 
    for (i=0; i<N_frames; i++) { 
        free((void*) tmp_sinograms[i]); 
        free((void*) tmp_sinograms_delay[i]); 
    }
    free((void*) global_sinogram); 
    free((void*) global_sinogram_delay);    
    // FIXME: destroy the data structures, except for the global data 

    globalobject.dynamic_projection->N_time_bins       = N_frames; 
    globalobject.dynamic_projection->N_counts          = N_events_total;
    globalobject.dynamic_projection->N_locations       = N_locations_dynamic; 
    globalobject.dynamic_projection->compression_ratio = (float) N_locations_global/ (float) (N_u*N_v*N_axial*N_azimuthal*N_frames); 
    globalobject.dynamic_projection->dynamic_inflation = (float) N_locations_dynamic/ (float) N_locations_global; 
    globalobject.dynamic_projection->listmode_loss     = (float) N_events_total/ (float) N_locations_dynamic; 
    globalobject.dynamic_projection->time_start        = time_start; 
    globalobject.dynamic_projection->time_end          = elapsed_time_ms;     

    globalobject.dynamic_projection_delay->N_time_bins       = N_frames; 
    globalobject.dynamic_projection_delay->N_counts          = N_events_total_delay;
    globalobject.dynamic_projection_delay->N_locations       = N_locations_dynamic_delay; 
    globalobject.dynamic_projection_delay->compression_ratio = (float) N_locations_global_delay/ (float) (N_u*N_v*N_axial*N_azimuthal*N_frames); 
    globalobject.dynamic_projection_delay->dynamic_inflation = (float) N_locations_dynamic_delay/ (float) N_locations_global_delay; 
    globalobject.dynamic_projection_delay->listmode_loss     = (float) N_events_total_delay/ (float) N_locations_dynamic_delay; 
    globalobject.dynamic_projection_delay->time_start        = time_start; 
    globalobject.dynamic_projection_delay->time_end          = elapsed_time_ms;     

    globalobject.has_projection = 1; 

    
    
    
    
    // Get information 
    StaticProjection *static_projection0 = globalobject.dynamic_projection->static_projections[0]; 
    unsigned int time_start0             = static_projection0->time_start; 
    unsigned int time_end0               = static_projection0->time_end; 
    unsigned int N_counts0               = static_projection0->N_counts; 
    unsigned int N_locations0            = static_projection0->N_locations;
    float compression_ratio0             = static_projection0->compression_ratio; 
    float listmode_loss0                 = static_projection0->listmode_loss;  

    unsigned int N_axial0                = static_projection0->layout->binning->N_axial; 
    unsigned int N_azimuthal0            = static_projection0->layout->binning->N_azimuthal; 
    float size_u0                        = static_projection0->layout->binning->size_u; 
    float size_v0                        = static_projection0->layout->binning->size_v; 
    unsigned int N_u0                    = static_projection0->layout->binning->N_u; 
    unsigned int N_v0                    = static_projection0->layout->binning->N_v; 
    
    fprintf(stderr,"NEVENTS FRAME0: %d\n",N_counts0);
    fprintf(stderr,"NLOCATIONS FRAME0: %d\n",N_locations0);
    
 
    StaticProjection *static_projection1 = globalobject.dynamic_projection->global_static_projection;  
    unsigned int time_start1             = static_projection1->time_start; 
    unsigned int time_end1               = static_projection1->time_end; 
    unsigned int N_counts1               = static_projection1->N_counts; 
    unsigned int N_locations1            = static_projection1->N_locations;
    float compression_ratio1             = static_projection1->compression_ratio; 
    float listmode_loss1                 = static_projection1->listmode_loss;  

    unsigned int N_axial1                = static_projection1->layout->binning->N_axial; 
    unsigned int N_azimuthal1            = static_projection1->layout->binning->N_azimuthal; 
    float size_u1                        = static_projection1->layout->binning->size_u; 
    float size_v1                        = static_projection1->layout->binning->size_v; 
    unsigned int N_u1                    = static_projection1->layout->binning->N_u; 
    unsigned int N_v1                    = static_projection1->layout->binning->N_v; 
    
    fprintf(stderr,"NEVENTS GLOBAL: %d\n",N_counts0);
    fprintf(stderr,"NLOCATIONS GLOBAL: %d\n",N_locations0);
    
    
    
    
    
    return status; 
}


extern int petlink32_load_gates(char *filename, int64_t *n_packets, unsigned int *use_time_bins, unsigned int *time_bins, callback_func_ptr callback)
{
    int status = STATUS_SUCCESS;                            // output status 
    int64_t N_packets  = *n_packets;
    unsigned int do_use_time_bins = *use_time_bins; 
    unsigned int time_lapse=0;
    unsigned int time_start=0;
    unsigned int time_end=0; 

    GatingData  *gating = instantiate_gating(MAX_GATING_EVENTS); 
    globalobject.gating = gating; 
    globalobject.has_gating_data = 1; 
    
    if (*use_time_bins!=0) {
        time_start = time_bins[0]; 
        time_end = time_bins[1]; 
        }
    
    Packet packet; 
    unsigned int buffer_size_batch = 1; //BUFFER_SIZE_BATCH; 
    int N_batches = floor(N_packets / buffer_size_batch) + 1; 
    int n_packets_last_batch = N_packets - (N_batches-1)*buffer_size_batch; 
    int n_packets_batch; 
    unsigned int done = 0;
    int64_t N_events_total    = 0; 
    unsigned int current_time_ms = 0; 
    unsigned int elapsed_time_ms = 0; 
    unsigned int waiting_for_firt_packet_time=1;
    unsigned int initial_time=0; 
    unsigned int i,j; 
    char batch_buffer[buffer_size_batch*BYTES_PER_PACKET]; 
    unsigned int triggers_counter=0; 
    unsigned int n_packets_total=0;

    fprintf(stderr,"N_packets: %d\n",N_packets);

    FILE *fid; 
    fid=fopen(filename, "rb");
    if (fid == NULL) {
        fprintf(stderr,"Failed to open listmode file. \n");
        status = STATUS_IO_ERROR; 
        return status; 
    }

    for (j = 0; j < N_batches; j++) {
        if(done) {break; }
        if (j==N_batches-1) {
            n_packets_batch = n_packets_last_batch; 
        }
        else {
            n_packets_batch = buffer_size_batch; 
        }
        status = read_packets_int(fid, batch_buffer, n_packets_batch); 
        n_packets_total ++;
        if (status!=STATUS_SUCCESS) {
            fprintf(stderr,"Problem reading the listmode data - end of file.\n"); 
            fprintf(stderr,"elapsed_time_ms: %d -- time_end: %d -- last time packet received: %d  -- start_time: %d \n",elapsed_time_ms,time_end,current_time_ms,time_start);
            fprintf(stderr,"n_packets_batch: %d\n", n_packets_batch);
            fprintf(stderr,"n_packets_total: %d\n", n_packets_total);
            fclose(fid); 
            //return status; 
            status=STATUS_SUCCESS; 
            done = 1; 
            break;
        }
        for (i = 0; i < n_packets_batch; i++) { 
            if (done) {break; }
            memcpy ( packet.buffer, batch_buffer+i*BYTES_PER_PACKET, BYTES_PER_PACKET );
            status = decode_packet(&packet);
            if (status!=STATUS_SUCCESS) {
                fprintf(stderr,"Problem decoding the listmode data.\n"); 
                fclose(fid); 
                return status; 
            }    
            if (packet.type == TYPE_TIME_ELAPSED) {
                if (waiting_for_firt_packet_time) {
                    initial_time = packet.time_ms; 
                    waiting_for_firt_packet_time = 0; 
                }
                current_time_ms = packet.time_ms; 
                elapsed_time_ms = packet.time_ms - initial_time; 
                // Time packet received
                //fprintf(stderr,"Elapsed time packet: %d [ms] \n",elapsed_time_ms);
                if (elapsed_time_ms >= time_end) {
                     // stop processing 
                     fprintf(stderr,"Done loading gates from listmode: elapsed_time_ms (%d) >= time_end (%d)  -- last time packet received: %d  -- start_time: %d \n",elapsed_time_ms,time_end,current_time_ms,time_start);
                     done = 1; 
                     callback(100); 
                     break; 
                }
            }
            else if (packet.type == TYPE_MONITORING_GATING0) {
                if (elapsed_time_ms>=time_start) {
                    gating->n_gating0 +=1; 
                    gating->n_events  +=1; 
                    gating->type[triggers_counter]=0;
                    gating->time[triggers_counter]=elapsed_time_ms;
                    gating->payload[triggers_counter]=packet.data;
                    triggers_counter  +=1; 
                }
            } 
            else if (packet.type == TYPE_MONITORING_GATING1) {
                if (elapsed_time_ms>=time_start) {
                    gating->n_gating1 +=1; 
                    gating->n_events  +=1; 
                    gating->type[triggers_counter]=1;
                    gating->time[triggers_counter]=elapsed_time_ms;
                    gating->payload[triggers_counter]=packet.data;
                    triggers_counter  +=1;
                } 
            } 
            else if (packet.type == TYPE_MONITORING_GATING2) {
                if (elapsed_time_ms>=time_start) {
                    gating->n_gating2 +=1; 
                    gating->n_events  +=1; 
                    gating->type[triggers_counter]=2;
                    gating->time[triggers_counter]=elapsed_time_ms;
                    gating->payload[triggers_counter]=packet.data;
                    triggers_counter  +=1; 
                }
            } 
            else if (packet.type == TYPE_MONITORING_GATING3) {
                if (elapsed_time_ms>=time_start) {
                    gating->n_gating3 +=1; 
                    gating->n_events  +=1; 
                    gating->type[triggers_counter]=3;
                    gating->time[triggers_counter]=elapsed_time_ms;
                    gating->payload[triggers_counter]=packet.data;
                    triggers_counter  +=1;
                }
            }                
            else if (packet.type == TYPE_MONITORING_GATING4) {
                if (elapsed_time_ms>=time_start) {
                    gating->n_gating4 +=1; 
                    gating->n_events  +=1; 
                    gating->type[triggers_counter]=4;
                    gating->time[triggers_counter]=elapsed_time_ms;
                    gating->payload[triggers_counter]=packet.data;
                    triggers_counter  +=1;
                }
            }   
            else if (packet.type == TYPE_MONITORING_GATING5) {
                if (elapsed_time_ms>=time_start) {
                    gating->n_gating5 +=1; 
                    gating->n_events  +=1; 
                    gating->type[triggers_counter]=5;
                    gating->time[triggers_counter]=elapsed_time_ms;
                    gating->payload[triggers_counter]=packet.data;
                    triggers_counter  +=1;
                }
            }   
            else if (packet.type == TYPE_MONITORING_GATING6) {
                if (elapsed_time_ms>=time_start) {
                    gating->n_gating6 +=1; 
                    gating->n_events  +=1; 
                    gating->type[triggers_counter]=6;
                    gating->time[triggers_counter]=elapsed_time_ms;
                    gating->payload[triggers_counter]=packet.data;
                    triggers_counter  +=1;
                }
            }   
            else if (packet.type == TYPE_MONITORING_GATING7) {
                if (elapsed_time_ms>=time_start) {
                    gating->n_gating7 +=1; 
                    gating->n_events  +=1; 
                    gating->type[triggers_counter]=7;
                    gating->time[triggers_counter]=elapsed_time_ms;
                    gating->payload[triggers_counter]=packet.data;
                    triggers_counter  +=1;
                }
                else {
                    fprintf(stderr,"Found gate 7 and elapsed_time_ms < time_start. \n");
                }
            }                
        } 
    }
    gating->search_time_start = time_start; 
    gating->search_time_end   = time_end; 
    return status; 
}


extern int get_gates_info(unsigned int *gating1, unsigned int *gating2, unsigned int *gating3, unsigned int *gating4, unsigned int *gating5, unsigned int *gating6, unsigned int *gating7, unsigned int *N, unsigned int *time_start, unsigned int *time_end)
{
    int status=STATUS_SUCCESS; 
    if (!globalobject.has_gating_data) {
        fprintf(stdout,"Gating data has not been loaded. \n");
        return STATUS_INITIALISATION_ERROR; 
    }
    GatingData *gating = globalobject.gating; 
    *gating1     = gating->n_gating1; 
    *gating2     = gating->n_gating2; 
    *gating3     = gating->n_gating3; 
    *gating4     = gating->n_gating4; 
    *gating5     = gating->n_gating5; 
    *gating6     = gating->n_gating6; 
    *gating7     = gating->n_gating7;
    *N             = gating->n_events; 
    *time_start    = gating->search_time_start; 
    *time_end      = gating->search_time_end; 
    return status;
}

extern int get_gates(unsigned int *N, unsigned int *type, unsigned int *time, unsigned int *payload)
{
    int status=STATUS_SUCCESS; 
    if (!globalobject.has_gating_data) {
        fprintf(stdout,"Gating data has not been loaded. \n");
        return STATUS_INITIALISATION_ERROR; 
    }
    // copy gating data
    unsigned int actual_N = *N; 
    if (actual_N > globalobject.gating->n_events)
        actual_N = globalobject.gating->n_events; 
    memcpy((void*) type, (void*) globalobject.gating->type, sizeof(unsigned int)* actual_N); 
    memcpy((void*) time, (void*) globalobject.gating->time, sizeof(unsigned int)* actual_N); 
    memcpy((void*) payload, (void*) globalobject.gating->payload, sizeof(unsigned int)* actual_N); 
    return status; 
}


/*
Get information about the listmode data 
*/
extern int get_petlink32_stats(int64_t *n_packets, unsigned int *n_unknown, unsigned int *n_prompt, unsigned int *n_delayed, unsigned int *n_elapsedtime, unsigned int *n_deadtime, unsigned int *n_motion, unsigned int *n_gating0, unsigned int *n_gating1, unsigned int *n_gating2, unsigned int *n_gating3, unsigned int *n_gating4, unsigned int *n_gating5, unsigned int *n_gating6, unsigned int *n_gating7, unsigned int *n_tracking, unsigned int *n_control_PET, unsigned int *n_control_MR, unsigned int *n_control_OTHER) 
{
    int status=STATUS_SUCCESS; 
    if (!globalobject.has_projection) {
        fprintf(stdout,"The dynamic sinogram has not been loaded. \n");
        return STATUS_INITIALISATION_ERROR; 
    }
    
    ListModeStats *stats = globalobject.stats; 
    *n_packets = stats->n_packets; 
    *n_unknown = stats->n_unknown; 
    *n_prompt  = stats->n_prompt; 
    *n_delayed = stats->n_delayed; 
    *n_elapsedtime = stats->n_elapsedtime; 
    *n_deadtime    = stats->n_deadtime; 
    *n_motion      = stats->n_motion; 
    *n_gating0     = stats->n_gating0; 
    *n_gating1     = stats->n_gating1; 
    *n_gating2     = stats->n_gating2; 
    *n_gating3     = stats->n_gating3;     
    *n_gating4     = stats->n_gating4; 
    *n_gating5     = stats->n_gating5; 
    *n_gating6     = stats->n_gating6; 
    *n_gating7     = stats->n_gating7;         
    *n_tracking    = stats->n_tracking; 
    *n_control_PET = stats->n_control_PET; 
    *n_control_MR  = stats->n_control_MR; 
    *n_control_OTHER = stats->n_control_OTHER;     
    return status; 
}


/*
Get information about the dynamic projection - prompts 
*/ 
extern int get_dynamic_projection_info_prompt(unsigned int *N_time_bins, int64_t *N_counts, unsigned int *N_locations, float *compression_ratio, float *dynamic_inflation, float *listmode_loss, unsigned int *time_start, unsigned int *time_end) 
{ 
    int status=STATUS_SUCCESS; 
    if (!globalobject.has_projection) {
        fprintf(stderr,"The dynamic sinogram has not been loaded. \n");
        return STATUS_INITIALISATION_ERROR; 
    }

    *N_time_bins = globalobject.dynamic_projection->N_time_bins; 
    *N_counts    = globalobject.dynamic_projection->N_counts; 
    *N_locations       = globalobject.dynamic_projection->N_locations;
    *compression_ratio = globalobject.dynamic_projection->compression_ratio; 
    *dynamic_inflation = globalobject.dynamic_projection->dynamic_inflation; 
    *listmode_loss     = globalobject.dynamic_projection->listmode_loss;  
    *time_start  = globalobject.dynamic_projection->time_start; 
    *time_end    = globalobject.dynamic_projection->time_end; 
    return status; 
}


/*
Get information about the dynamic projection - delays  
*/ 
extern int get_dynamic_projection_info_delay(unsigned int *N_time_bins, int64_t *N_counts, unsigned int *N_locations, float *compression_ratio, float *dynamic_inflation, float *listmode_loss, unsigned int *time_start, unsigned int *time_end) 
{ 
    int status=STATUS_SUCCESS; 
    if (!globalobject.has_projection) {
        fprintf(stderr,"The dynamic sinogram has not been loaded. \n");
        return STATUS_INITIALISATION_ERROR; 
    }

    *N_time_bins = globalobject.dynamic_projection_delay->N_time_bins; 
    *N_counts    = globalobject.dynamic_projection_delay->N_counts; 
    *N_locations       = globalobject.dynamic_projection_delay->N_locations;
    *compression_ratio = globalobject.dynamic_projection_delay->compression_ratio; 
    *dynamic_inflation = globalobject.dynamic_projection_delay->dynamic_inflation; 
    *listmode_loss     = globalobject.dynamic_projection_delay->listmode_loss;  
    *time_start  = globalobject.dynamic_projection_delay->time_start; 
    *time_end    = globalobject.dynamic_projection_delay->time_end; 
    return status; 
}



/*
Get information about a static projection - prompts 
*/ 
extern int get_static_projection_info_prompt(unsigned int *time_bin, unsigned int *time_start, unsigned int *time_end, unsigned int *N_counts, unsigned int *N_locations, float *compression_ratio, float *listmode_loss, unsigned int *N_axial, unsigned int *N_azimuthal, float *angles_axial, float *angles_azimuthal, float *size_u, float *size_v, unsigned int *N_u, unsigned int *N_v) 
{
    int status=STATUS_SUCCESS; 
    if (!globalobject.has_projection) {
        fprintf(stderr,"The dynamic sinogram has not been loaded. \n");
        return STATUS_INITIALISATION_ERROR; 
    }
    
    // Check if the requested time bin exists
    unsigned int Time_bin = *time_bin; 
    if (Time_bin >= globalobject.dynamic_projection->N_time_bins) {
        fprintf(stderr,"The requested time bin %d does not exist: there are %d time bins. \n", Time_bin, globalobject.dynamic_projection->N_time_bins); 
        return STATUS_PARAMETER_ERROR; 
    }
    
    // Get information 
    StaticProjection *static_projection = globalobject.dynamic_projection->static_projections[Time_bin]; 
    *time_start             = static_projection->time_start; 
    *time_end               = static_projection->time_end; 
    *N_counts               = static_projection->N_counts; 
    *N_locations            = static_projection->N_locations;
    *compression_ratio      = static_projection->compression_ratio; 
    *listmode_loss          = static_projection->listmode_loss;  

    *N_axial                = static_projection->layout->binning->N_axial; 
    *N_azimuthal            = static_projection->layout->binning->N_azimuthal; 
    *size_u                 = static_projection->layout->binning->size_u; 
    *size_v                 = static_projection->layout->binning->size_v; 
    *N_u                    = static_projection->layout->binning->N_u; 
    *N_v                    = static_projection->layout->binning->N_v; 
    
//    memcpy((void*) angles_axial, (void*) static_projection->layout->binning->angles_axial, sizeof(float) * static_projection->layout->binning->N_axial); 
//    memcpy((void*) angles_azimuthal, (void*) static_projection->layout->binning->angles_azimuthal, sizeof(float) * static_projection->layout->binning->N_azimuthal); 
 
    return status;     
}



/*
Get information about a static projection - delays
*/ 
extern int get_static_projection_info_delay(unsigned int *time_bin, unsigned int *time_start, unsigned int *time_end, unsigned int *N_counts, unsigned int *N_locations, float *compression_ratio, float *listmode_loss, unsigned int *N_axial, unsigned int *N_azimuthal, float *angles_axial, float *angles_azimuthal, float *size_u, float *size_v, unsigned int *N_u, unsigned int *N_v) 
{
    int status=STATUS_SUCCESS; 
    if (!globalobject.has_projection) {
        fprintf(stderr,"The dynamic sinogram has not been loaded. \n");
        return STATUS_INITIALISATION_ERROR; 
    }
    
    // Check if the requested time bin exists
    unsigned int Time_bin = *time_bin; 
    if (Time_bin >= globalobject.dynamic_projection_delay->N_time_bins) {
        fprintf(stderr,"The requested time bin %d does not exist: there are %d time bins. \n", Time_bin, globalobject.dynamic_projection_delay->N_time_bins); 
        return STATUS_PARAMETER_ERROR; 
    }
    
    // Get information 
    StaticProjection *static_projection = globalobject.dynamic_projection_delay->static_projections[Time_bin]; 
    *time_start             = static_projection->time_start; 
    *time_end               = static_projection->time_end; 
    *N_counts               = static_projection->N_counts; 
    *N_locations            = static_projection->N_locations;
    *compression_ratio      = static_projection->compression_ratio; 
    *listmode_loss          = static_projection->listmode_loss;  

    *N_axial                = static_projection->layout->binning->N_axial; 
    *N_azimuthal            = static_projection->layout->binning->N_azimuthal; 
    *size_u                 = static_projection->layout->binning->size_u; 
    *size_v                 = static_projection->layout->binning->size_v; 
    *N_u                    = static_projection->layout->binning->N_u; 
    *N_v                    = static_projection->layout->binning->N_v; 
    
//    memcpy((void*) angles_axial, (void*) static_projection->layout->binning->angles_axial, sizeof(float) * static_projection->layout->binning->N_axial); 
//    memcpy((void*) angles_azimuthal, (void*) static_projection->layout->binning->angles_azimuthal, sizeof(float) * static_projection->layout->binning->N_azimuthal); 
 
    return status;     
}


/*
Get information about the global static projection - prompts
*/ 
extern int get_global_static_projection_info_prompt(unsigned int *time_start, unsigned int *time_end, unsigned int *N_counts, unsigned int *N_locations, float *compression_ratio, float *listmode_loss, unsigned int *N_axial, unsigned int *N_azimuthal, float *angles_axial, float *angles_azimuthal, float *size_u, float *size_v, unsigned int *N_u, unsigned int *N_v) 
{
    int status=STATUS_SUCCESS; 
    if (!globalobject.has_projection) {
        fprintf(stderr,"The dynamic sinogram has not been loaded. \n");
        return STATUS_INITIALISATION_ERROR; 
    }
        
    // Get information 
    StaticProjection *static_projection = globalobject.dynamic_projection->global_static_projection; 
    *time_start             = static_projection->time_start; 
    *time_end               = static_projection->time_end; 
    *N_counts               = static_projection->N_counts; 
    *N_locations            = static_projection->N_locations;
    *compression_ratio      = static_projection->compression_ratio; 
    *listmode_loss          = static_projection->listmode_loss;  

    *N_axial                = static_projection->layout->binning->N_axial; 
    *N_azimuthal            = static_projection->layout->binning->N_azimuthal; 

    *size_u                 = static_projection->layout->binning->size_u; 
    *size_v                 = static_projection->layout->binning->size_v; 
    *N_u                    = static_projection->layout->binning->N_u; 
    *N_v                    = static_projection->layout->binning->N_v; 
    
//    memcpy((void*) angles_axial, (void*) static_projection->layout->binning->angles_axial, sizeof(float) * static_projection->layout->binning->N_axial); 
//    memcpy((void*) angles_azimuthal, (void*) static_projection->layout->binning->angles_azimuthal, sizeof(float) * static_projection->layout->binning->N_azimuthal); 

    return status;     
}



/*
Get information about the global static projection - delays
*/ 
extern int get_global_static_projection_info_delay(unsigned int *time_start, unsigned int *time_end, unsigned int *N_counts, unsigned int *N_locations, float *compression_ratio, float *listmode_loss, unsigned int *N_axial, unsigned int *N_azimuthal, float *angles_axial, float *angles_azimuthal, float *size_u, float *size_v, unsigned int *N_u, unsigned int *N_v) 
{
    int status=STATUS_SUCCESS; 
    if (!globalobject.has_projection) {
        fprintf(stderr,"The dynamic sinogram has not been loaded. \n");
        return STATUS_INITIALISATION_ERROR; 
    }
        
    // Get information 
    StaticProjection *static_projection = globalobject.dynamic_projection_delay->global_static_projection; 
    *time_start             = static_projection->time_start; 
    *time_end               = static_projection->time_end; 
    *N_counts               = static_projection->N_counts; 
    *N_locations            = static_projection->N_locations;
    *compression_ratio      = static_projection->compression_ratio; 
    *listmode_loss          = static_projection->listmode_loss;  

    *N_axial                = static_projection->layout->binning->N_axial; 
    *N_azimuthal            = static_projection->layout->binning->N_azimuthal; 

    *size_u                 = static_projection->layout->binning->size_u; 
    *size_v                 = static_projection->layout->binning->size_v; 
    *N_u                    = static_projection->layout->binning->N_u; 
    *N_v                    = static_projection->layout->binning->N_v; 
    
//    memcpy((void*) angles_axial, (void*) static_projection->layout->binning->angles_axial, sizeof(float) * static_projection->layout->binning->N_axial); 
//    memcpy((void*) angles_azimuthal, (void*) static_projection->layout->binning->angles_azimuthal, sizeof(float) * static_projection->layout->binning->N_azimuthal); 

    return status;     
}



/*
Extract the static projection for a given time bin from the dynamic projection - prompts 
*/
extern int get_static_projection_prompt(unsigned int *time_bin, unsigned int *time_start, unsigned int *time_end, unsigned int *N_counts, unsigned int *N_locations, float *compression_ratio, float *listmode_loss, unsigned int *N_axial, unsigned int *N_azimuthal, float *angles_axial, float *angles_azimuthal, float *size_u, float *size_v, unsigned int *N_u, unsigned int *N_v, int *offsets_matrix, float *counts, unsigned short *locations) 
{
    int status=STATUS_SUCCESS; 
    if (!globalobject.has_projection) {
        fprintf(stderr,"The dynamic sinogram has not been loaded. \n");
        return STATUS_INITIALISATION_ERROR; 
    }
    
    // Check if the requested time bin exists
    unsigned int Time_bin = *time_bin; 
    if (Time_bin >= globalobject.dynamic_projection->N_time_bins) {
        fprintf(stderr,"The requested time bin %d does not exist: there are %d time bins. \n", Time_bin, globalobject.dynamic_projection->N_time_bins); 
        return STATUS_PARAMETER_ERROR; 
    }
    
    // Get information 
    StaticProjection *static_projection = globalobject.dynamic_projection->static_projections[Time_bin]; 
    *time_start             = static_projection->time_start; 
    *time_end               = static_projection->time_end; 
    *N_counts               = static_projection->N_counts; 
    *N_locations            = static_projection->N_locations;
    *compression_ratio      = static_projection->compression_ratio; 
    *listmode_loss          = static_projection->listmode_loss;  
    
    *N_axial                = static_projection->layout->binning->N_axial; 
    *N_azimuthal            = static_projection->layout->binning->N_azimuthal; 

    *size_u                 = static_projection->layout->binning->size_u; 
    *size_v                 = static_projection->layout->binning->size_v; 
    *N_u                    = static_projection->layout->binning->N_u; 
    *N_v                    = static_projection->layout->binning->N_v; 

//    memcpy((void*) angles_axial, (void*) static_projection->layout->binning->angles_axial, sizeof(float) * static_projection->layout->binning->N_axial); 
//    memcpy((void*) angles_azimuthal, (void*) static_projection->layout->binning->angles_azimuthal, sizeof(float) * static_projection->layout->binning->N_azimuthal); 

    memcpy((void*) offsets_matrix, (void*) static_projection->layout->offsets->matrix, sizeof(unsigned int) * static_projection->layout->binning->N_axial * static_projection->layout->binning->N_azimuthal); 
    memcpy((void*) counts, (void*) static_projection->counts, sizeof(float) * static_projection->N_locations ); 
    memcpy((void*) locations, (void*) static_projection->locations, sizeof(unsigned short) * 3 * static_projection->N_locations ); 
    return status;     
}



/*
Extract the static projection for a given time bin from the dynamic projection - delays 
*/
extern int get_static_projection_delay(unsigned int *time_bin, unsigned int *time_start, unsigned int *time_end, unsigned int *N_counts, unsigned int *N_locations, float *compression_ratio, float *listmode_loss, unsigned int *N_axial, unsigned int *N_azimuthal, float *angles_axial, float *angles_azimuthal, float *size_u, float *size_v, unsigned int *N_u, unsigned int *N_v, int *offsets_matrix, float *counts, unsigned short *locations) 
{
    int status=STATUS_SUCCESS; 
    if (!globalobject.has_projection) {
        fprintf(stderr,"The dynamic sinogram has not been loaded. \n");
        return STATUS_INITIALISATION_ERROR; 
    }
    
    // Check if the requested time bin exists
    unsigned int Time_bin = *time_bin; 
    if (Time_bin >= globalobject.dynamic_projection_delay->N_time_bins) {
        fprintf(stderr,"The requested time bin %d does not exist: there are %d time bins. \n", Time_bin, globalobject.dynamic_projection_delay->N_time_bins); 
        return STATUS_PARAMETER_ERROR; 
    }
    
    // Get information 
    StaticProjection *static_projection = globalobject.dynamic_projection_delay->static_projections[Time_bin]; 
    *time_start             = static_projection->time_start; 
    *time_end               = static_projection->time_end; 
    *N_counts               = static_projection->N_counts; 
    *N_locations            = static_projection->N_locations;
    *compression_ratio      = static_projection->compression_ratio; 
    *listmode_loss          = static_projection->listmode_loss;  
    
    *N_axial                = static_projection->layout->binning->N_axial; 
    *N_azimuthal            = static_projection->layout->binning->N_azimuthal; 

    *size_u                 = static_projection->layout->binning->size_u; 
    *size_v                 = static_projection->layout->binning->size_v; 
    *N_u                    = static_projection->layout->binning->N_u; 
    *N_v                    = static_projection->layout->binning->N_v; 

//    memcpy((void*) angles_axial, (void*) static_projection->layout->binning->angles_axial, sizeof(float) * static_projection->layout->binning->N_axial); 
//    memcpy((void*) angles_azimuthal, (void*) static_projection->layout->binning->angles_azimuthal, sizeof(float) * static_projection->layout->binning->N_azimuthal); 

    memcpy((void*) offsets_matrix, (void*) static_projection->layout->offsets->matrix, sizeof(unsigned int) * static_projection->layout->binning->N_axial * static_projection->layout->binning->N_azimuthal); 
    memcpy((void*) counts, (void*) static_projection->counts, sizeof(float) * static_projection->N_locations ); 
    memcpy((void*) locations, (void*) static_projection->locations, sizeof(unsigned short) * 3 * static_projection->N_locations ); 
    return status;     
}



/*
Extract the global static projection - prompts
*/
extern int get_global_static_projection_prompt(unsigned int *time_start, unsigned int *time_end, unsigned int *N_counts, unsigned int *N_locations, float *compression_ratio, float *listmode_loss, unsigned int *N_axial, unsigned int *N_azimuthal, float *angles_axial, float *angles_azimuthal, float *size_u, float *size_v, unsigned int *N_u, unsigned int *N_v, int *offsets_matrix, float *counts, unsigned short *locations) 
{
    int status=STATUS_SUCCESS; 
    if (!globalobject.has_projection) {
        fprintf(stderr,"The dynamic sinogram has not been loaded. \n");
        return STATUS_INITIALISATION_ERROR; 
    }

    // Get information 
    StaticProjection *static_projection = globalobject.dynamic_projection->global_static_projection;
    *time_start             = static_projection->time_start; 
    *time_end               = static_projection->time_end;
    *N_counts               = static_projection->N_counts; 
    *N_locations            = static_projection->N_locations;
    *compression_ratio      = static_projection->compression_ratio; 
    *listmode_loss          = static_projection->listmode_loss;  

    *N_axial                = static_projection->layout->binning->N_axial; 
    *N_azimuthal            = static_projection->layout->binning->N_azimuthal; 

    *size_u                 = static_projection->layout->binning->size_u; 
    *size_v                 = static_projection->layout->binning->size_v; 
    *N_u                    = static_projection->layout->binning->N_u; 
    *N_v                    = static_projection->layout->binning->N_v; 

//    memcpy((void*) angles_axial, (void*) static_projection->layout->binning->angles_axial, sizeof(float) * static_projection->layout->binning->N_axial); 
//    memcpy((void*) angles_azimuthal, (void*) static_projection->layout->binning->angles_azimuthal, sizeof(float) * static_projection->layout->binning->N_azimuthal); 

    memcpy((void*) offsets_matrix, (void*) static_projection->layout->offsets->matrix, sizeof(unsigned int) * static_projection->layout->binning->N_axial * static_projection->layout->binning->N_azimuthal); 
    memcpy((void*) counts, (void*) static_projection->counts, sizeof(float) * static_projection->N_locations ); 
    memcpy((void*) locations, (void*) static_projection->locations, sizeof(unsigned short) * 3 * static_projection->N_locations ); 
    return status;     
}


/*
Extract the global static projection - delays
*/
extern int get_global_static_projection_delay(unsigned int *time_start, unsigned int *time_end, unsigned int *N_counts, unsigned int *N_locations, float *compression_ratio, float *listmode_loss, unsigned int *N_axial, unsigned int *N_azimuthal, float *angles_axial, float *angles_azimuthal, float *size_u, float *size_v, unsigned int *N_u, unsigned int *N_v, int *offsets_matrix, float *counts, unsigned short *locations) 
{
    int status=STATUS_SUCCESS; 
    if (!globalobject.has_projection) {
        fprintf(stderr,"The dynamic sinogram has not been loaded. \n");
        return STATUS_INITIALISATION_ERROR; 
    }

    // Get information 
    StaticProjection *static_projection = globalobject.dynamic_projection_delay->global_static_projection; 
    *time_start             = static_projection->time_start; 
    *time_end               = static_projection->time_end; 
    *N_counts               = static_projection->N_counts; 
    *N_locations            = static_projection->N_locations;
    *compression_ratio      = static_projection->compression_ratio; 
    *listmode_loss          = static_projection->listmode_loss;  

    *N_axial                = static_projection->layout->binning->N_axial; 
    *N_azimuthal            = static_projection->layout->binning->N_azimuthal; 

    *size_u                 = static_projection->layout->binning->size_u; 
    *size_v                 = static_projection->layout->binning->size_v; 
    *N_u                    = static_projection->layout->binning->N_u; 
    *N_v                    = static_projection->layout->binning->N_v; 

//    memcpy((void*) angles_axial, (void*) static_projection->layout->binning->angles_axial, sizeof(float) * static_projection->layout->binning->N_axial); 
//    memcpy((void*) angles_azimuthal, (void*) static_projection->layout->binning->angles_azimuthal, sizeof(float) * static_projection->layout->binning->N_azimuthal); 

    memcpy((void*) offsets_matrix, (void*) static_projection->layout->offsets->matrix, sizeof(unsigned int) * static_projection->layout->binning->N_axial * static_projection->layout->binning->N_azimuthal); 
    memcpy((void*) counts, (void*) static_projection->counts, sizeof(float) * static_projection->N_locations ); 
    memcpy((void*) locations, (void*) static_projection->locations, sizeof(unsigned short) * 3 * static_projection->N_locations ); 
    return status;     
}



/*
Free memory
*/
extern int free_memory(void)
{
    int status = STATUS_SUCCESS; 
    deallocate_globalobject(); 
    return status;
}
