#include "debug.h"
#include "key.h"
#include "lcd.h"
#include <stdio.h>
float Temp_Para=450;

float Template_para = 0;
ParameterNode_Type P_Ctrl[20];//常调参数

extern float test_para1;
extern float test_para2;
void ParameterList_Init(void)
{
	ParameterNode_Type node;
	ParameterNode_Type *blank_node;
	ParameterList_Type insert_place=0;
	blank_node=P_Ctrl;
	
//    node.exterdata=&(Template_para);
//	node.step=1;
//	sprintf(node.name,"Template_para"); 
//	Insert_ParameterNode(&node,&insert_place,blank_node++);
    
	node.exterdata=&(test_para1);
	node.step=1;
	sprintf(node.name,"test_para1"); 
	Insert_ParameterNode(&node,&insert_place,blank_node++);
	
	node.exterdata=&(test_para2);
	node.step=1;
	sprintf(node.name,"test_para2"); 
	Insert_ParameterNode(&node,&insert_place,blank_node++);
    

    

	Show_Parameter(P_Ctrl,0x01|0x02|0x04);
}

void Data_Init(void)
{
	/*不可调参数*/
    ;
}

void Parameter_Change(void)
{
	static ParameterNode_Type* para=P_Ctrl;
	uint8_t keysta;
	keysta=Key_STA();

	if(keysta&Key_3_MASK)
	{
        para=para->prepara;
        Show_Parameter(para,0x01|0x02|0x04);
	}
	
	if(keysta&Key_2_MASK)
	{
        para=para->nextpara;
        Show_Parameter(para,0x01|0x02|0x04);
	}
	
	if(keysta&Key_4_MASK)
	{
		*(para->exterdata)+=para->step;
		Show_Parameter(para,2);
	}
	
	if(keysta&Key_1_MASK)
	{
		*(para->exterdata)-=para->step;
		Show_Parameter(para,2);
	}

}

//config
//bit0:	显示参数名
//bit1:	显示参数
//bit2:	显示步进值
void Show_Parameter(ParameterNode_Type *para, uint8_t config)
{
	char str[10];
	
	//显示参数名
	if(config&0x01)
	{
		sprintf(str,"%-9s",para->name);
        LCD_ShowString(0,370,210,24,24,(uint8_t*)str); 
	}
	//显示参数
	if(config&0x02)
	{
		sprintf(str,"%-9.2f",*(para->exterdata));
		LCD_ShowString(0,430,210,24,24,(uint8_t*)str); 
	}
	//显示步进值
	if(config&0x04)
	{
		sprintf(str,"step: %-9.2f",para->step);
		LCD_ShowString(0,400,210,24,24,(uint8_t*)str); 
	}
}


//在insert_place节点后插入新节点,并使insert_place指向新插入的节点
static void Insert_ParameterNode(ParameterNode_Type *insert_node, ParameterList_Type *insert_place, ParameterNode_Type *blank_node)
{
    uint8_t i = 0;
	//复制数据
	blank_node->exterdata=insert_node->exterdata;
	for(i=0;i<ParameterName_Size;i++)
	{
		blank_node->name[i]=insert_node->name[i];
	}
	blank_node->step=insert_node->step;
	
	//插入节点
	if((*insert_place)!=0)
	{
		blank_node->nextpara=(*insert_place)->nextpara;
		((*insert_place)->nextpara)->prepara=blank_node;
		(*insert_place)->nextpara=blank_node;
		blank_node->prepara=(*insert_place);
	}
	else
	{
		blank_node->nextpara=blank_node;
		blank_node->prepara=blank_node;
	}
	(*insert_place)=blank_node;
}
