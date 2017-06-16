#include "mental_sensor.h"
#include "LDC1614.h"

//ÏßÈ¦
uint32_t L1=0,L2=0,L3=0,L4=0,
   L1_min=0,L2_min=0,L3_min=0,L4_min=0,
   L1_min_=0,L2_min_=0,L3_min_=0,L4_min_=0,
   L1_max=0,L2_max=0,L3_max=0,L4_max=0,
   R1=0,R2=0,R3=0,R4=0,
   R1_min=0,R2_min=0,R3_min=0,R4_min=0,
   R1_min_=0,R2_min_=0,R3_min_=0,R4_min_=0,
   R1_max=0,R2_max=0,R3_max=0,R4_max=0;

int16_t  L1_e=0,L2_e=0,L3_e=0,L4_e=0,
   R1_e=0,R2_e=0,R3_e=0,R4_e=0,
   L1_E=0,L2_E=0,L3_E=0,L4_E=0,
   R1_E=0,R2_E=0,R3_E=0,R4_E=0,
   L1_ee=0,L2_ee=0,L3_ee=0,L4_ee=0,
   R1_ee=0,R2_ee=0,R3_ee=0,R4_ee=0,
   LL=0,RR=0;
int16_t L3_coil[10]={0},L4_coil[10]={0},R3_coil[10]={0},R4_coil[10]={0};
int16_t  error=0,error_=0,Test_Flag=0;

int16_t coil[8]={0},COIL[8]={0};
int16_t Tx=0,Left=0,Right=0;
int16_t ERR=0;

int16_t Flag_S=0,Flag_D=0,shizi=0,Flag_A=0,P_Flag=0;

//??
//error???????
//float p1=10;???
//float q1=0.0001;????
 //float r1=0.0005;????
double p[8]={10,10,10,10,10,10,10,10}; 
double q[8]={10,2.5,2.5,2.5,1.5,2.5,2.5,2.5};
double r[8]={30,25,10,10,15,20,10,10};
int16_t error_slope[8];

int16_t slope_kalman_filter(int16_t slope,int16_t mark_num);

void getline_L(uint8_t * flag)
{    
    int i;
    static int Test_Flag=0;
    if(*flag==1)
    {
         //PTC4_O=0;
         L1  = LDC1614_1_GetResult( DATA_LSB_CH0 )/1000;
         L2  = LDC1614_1_GetResult( DATA_LSB_CH1 )/1000;
         L3  = LDC1614_1_GetResult( DATA_LSB_CH2 )/1000;
         L4  = LDC1614_1_GetResult( DATA_LSB_CH3 )/1000;
         //state=LDC1614_1_GetResult( STATUS );
         LDC1614_1_WriteReg(CONFIG, 0x34, 0x01); //ldc1??
         LDC1614_2_WriteReg(CONFIG, 0x14, 0x01); //ldc2??  
     }
     else if(*flag>=2)
     {
         *flag=0;
         R4  = LDC1614_2_GetResult( DATA_LSB_CH0 )/1000;
         R3  = LDC1614_2_GetResult( DATA_LSB_CH1 )/1000;
         R2  = LDC1614_2_GetResult( DATA_LSB_CH2 )/1000;
         R1  = LDC1614_2_GetResult( DATA_LSB_CH3 )/1000;
         //state=LDC1614_2_GetResult( STATUS );
         LDC1614_2_WriteReg(CONFIG, 0x34, 0x01); //ldc2??
         LDC1614_1_WriteReg(CONFIG, 0x14, 0x01); //ldc1??      
     } 
         
     if(Test_Flag<50){
        if(Test_Flag<10){           
              L1_min_=L1;         
              L2_min_=L2;
              L3_min_=L3;
              L4_min_=L4;
              R1_min_=R1;         
              R2_min_=R2;
              R3_min_=R3;
              R4_min_=R4;  
             
             
         }
         else{  
              L1_min_+=L1;         
              L2_min_+=L2;
              L3_min_+=L3;
              L4_min_+=L4;
              R1_min_+=R1;         
              R2_min_+=R2;
              R3_min_+=R3;
              R4_min_+=R4;
              
         } 
         Test_Flag++;
         return;
     }else if(Test_Flag==50) {
          L1_min=(int32_t)(L1_min_/41);
          L2_min=(int32_t)(L2_min_/41);
          L3_min=(int32_t)(L3_min_/41);
          L4_min=(int32_t)(L4_min_/41);
          
          R1_min=(int32_t)(R1_min_/41);
          R2_min=(int32_t)(R2_min_/41);
          R3_min=(int32_t)(R3_min_/41);
          R4_min=(int32_t)(R4_min_/41);  
          

          Test_Flag++;
      }
        
     L1_e=L1-L1_min;
     L2_e=L2-L2_min;
     L3_e=L3-L3_min;
     L4_e=L4-L4_min;
     
     R1_e=R1-R1_min;
     R2_e=R2-R2_min;
     R3_e=R3-R3_min;
     R4_e=R4-R4_min;
     
      if(L1_e<55 && L1_e>-5)
           L1_E=(int16_t)(20*L1_e/26);
      if(L2_e<55 && L2_e>-5)
           L2_E=(int16_t)(20*L2_e/24);
      if(L3_e<55 && L3_e>-5)
           L3_E=(int16_t)(20*L3_e/22);
      if(L4_e<55 && L4_e>-5)
           L4_E=(int16_t)(20*L4_e/20);
     
      if(R1_e<55 && R1_e>-5)
           R1_E=(int16_t)(20*R1_e/28);
      if(R2_e<55 && R2_e>-5)
           R2_E=(int16_t)(20*R2_e/30);
      if(R3_e<55 && R3_e>-5)
           R3_E=(int16_t)(20*R3_e/30);
      if(R4_e<55 && R4_e>-5)
           R4_E=(int16_t)(20*R4_e/26);

      

      
      coil[0]=L4_E;
      coil[1]=L3_E;
      coil[2]=L2_E;
      coil[3]=L1_E;
      coil[4]=R1_E;
      coil[5]=R2_E;
      coil[6]=R3_E;
      coil[7]=R4_E;
      //???
      for( i=0;i<8;i++){
          if(coil[i]>10) COIL[i]=1;
          else COIL[i]=0;
      }
      //L2_E??
//      if(COIL[2]==1 && COIL[1]==0 && COIL[3]==0) COIL[2]=0,L2_E=5;
      //????????
  /*    for(int i=1;i<7;i++){
          if(COIL[i-1]==0 && COIL[i]==1 && COIL[i+1]==0) COIL[i]=0,coil[i]=5;
      }
      for(int i=1;i<7;i++){
          if(COIL[i-1]==1 && COIL[i]==0 && COIL[i+1]==1) COIL[i]=1,coil[i]=15;
      }
      
      if(COIL[1]+COIL[2]+COIL[3]+COIL[4]+COIL[5]+COIL[6]>=2 && (COIL[0]==1 && COIL[1]==0)) COIL[0]=0,coil[0]=5;
      if(COIL[1]+COIL[2]+COIL[3]+COIL[4]+COIL[5]+COIL[6]>=2 && (COIL[7]==1 && COIL[6]==0)) COIL[7]=0,coil[7]=5;
      //??*/
      
      for(i=0;i<9;i++) 
      {
        L3_coil[i]=L3_coil[i+1];
        L4_coil[i]=L4_coil[i+1];
        R3_coil[i]=R3_coil[i+1];
        R4_coil[i]=R4_coil[i+1];
        
      }
      L3_coil[9]=COIL[1];
      L4_coil[9]=COIL[0];
      R3_coil[9]=COIL[6];
      R4_coil[9]=COIL[7];
       
      Flag_S=0;
      for( i=0;i<10;i++)
      {
        if(L3_coil[i]==1 ||L4_coil[i]==1 ||R3_coil[i]==1 ||R4_coil[i]==1 )
        {
          Flag_S++;
        }
        
      }
      if(Flag_S<=3 && COIL[3]==1 && COIL[4]==1) Flag_D=0;
      else Flag_D=1;
      
      
      for( i=0;i<7;i++){
          if(COIL[i]-COIL[i+1]<0){
              //Tx=1;
              Left=i;
          }
          else if(COIL[7-i]-COIL[6-i]<0){
              //Tx=1;
              Right=7-i;
          }
      }
      
      if(COIL[0]==0 && Left==0 && Right==1) Right=0;
      else if(COIL[7]==0 && Left==6 && Right==7) Left=7;
      
//      if(Right<=Left) {
          if((COIL[0]+COIL[1])==2 || (COIL[0]==1 && COIL[1]==0)) Left=0;
          //if((COIL[6]+COIL[7])==2 || (COIL[7]==1 && COIL[6]==0)) Right=0;
//     }
//     if(Right>=Left) {
          else if((COIL[6]+COIL[7])==2 || (COIL[7]==1 && COIL[6]==0)) Right=7;
//     }

           
      if(Right>=Left) ERR=Right+Left;
      
      
            
      if((COIL[0]==1 && COIL[7]==1)|| (COIL[0]==1 && COIL[6]==1)||(COIL[1]==1 && COIL[7]==1)) 
      {
        shizi=10;
        error=0;
        Flag_D=4;
      }
      else shizi--;
      
      if(shizi<=0){
        shizi=0;
        
            switch(ERR){
          case 0:{
              error=120-coil[0]; };break;
          case 1:{                                       
              error=115-coil[1]-coil[0]; };break;
          case 2:{                                       
              error=100-coil[1]-coil[2]; };break;
          case 3:{                                       
              error=(85-coil[3]-coil[2]);             };break;
          case 4:{                                       
              error=(coil[0]-coil[3]+60);   };break;
          case 5:{                                       
              error=(coil[1]-coil[4]+25);            };break;
          case 6:{                                       
              error=coil[2]-coil[5];                  };break;
          case 7:{                                       
              error=coil[2]-coil[5];                  };break;
          case 8:{                                       
              error=coil[2]-coil[5];                  };break;
          case 9:{                                       
              error=(coil[3]-coil[6]-25);             };break;
          case 10:{                                      
              error=(coil[4]-coil[7]-60);   };break;
          case 11:{                                      
              error=(coil[4]+coil[5]-85);             };break;
          case 12:{                                      
              error=coil[5]+coil[6]-100; };break;   //y = 9.5e-06*x^{2} - 9.8e-06*x + 0.52
          case 13:{                                                         //y = 8.5e-06*x^{2} - 0.00021*x + 0.52
              error=coil[6]+coil[7]-115; };break;
          case 14:{                                      
              error=coil[7]-120; };break;
          default:break;
      }
      
      }
      else error=(int16_t)(0.5*(coil[2]-coil[5]+coil[1]-coil[6]));
        
   /*-------------------------?S----------------------------*/   
//      for(int i=0;i<49;i++)
//      {
//        S_error[i]=S_error[i+1];
//      }
//      S_error[49]=error;
//      
//      S_Z=0;
//      S_P=0;
//      S_N=0;
//      for(int i=0;i<50;i++)
//      {
//        if(S_error[i]>50) S_P++;
//        else{
//          if(S_error[i]<-50) S_N++;
//          else S_Z++;
//        }
//      }
//      
//      if(S_P>13 && S_Z>7 && S_N>13) S_Flag++;
//      else S_Flag--;
//      if(S_Flag>20) S_Flag=20;
//      if(S_Flag<0) S_Flag=0;
//      
//      if(S_Flag>4) Flag_D=6;
    /*---------------------------??--------------------------------------*/  
   /*   if((L1_e>150 || R1_e>150) && (abs(ERR-7)<=2) &&
         Flag_A!=1 && Flag_A!=2 && Flag_D==0) Flag_D=5,Flag_A=1;
      if(Flag_A==1) 
      {
         error=(int16_t)(0.4*error);
//        if((COIL[0]+COIL[1]+COIL[2]+COIL[3]+COIL[4]+COIL[5]+COIL[6]+COIL[7])<2) error=(int16_t)(0.5*(-Last_error));
//        else Last_error=error;
      }
          */
          
          error = slope_kalman_filter(error,0);
}



int16_t slope_kalman_filter(int16_t slope,int16_t mark_num)
{
    float kGain=0;
    int16_t Last_slope;
    Last_slope=error_slope[mark_num];
    p[mark_num]= p[mark_num]+q[mark_num];       
    kGain = p[mark_num]/(p[mark_num]+r[mark_num]);
    slope= Last_slope + (int16_t)(kGain*(slope-Last_slope));
    p[mark_num]= (1-kGain)*p[mark_num];
    error_slope[mark_num]= slope;
    return slope;
}



