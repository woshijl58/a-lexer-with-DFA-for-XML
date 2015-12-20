#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct
{
char *p;
int len;
}
xml_Text;

typedef enum {
xml_tt_U, /* Unknow */
xml_tt_H, /* XML Head <?xxx?>*/
xml_tt_E, /* End Tag </xxx> */
xml_tt_B, /* Start Tag <xxx> */
xml_tt_BE, /* Tag <xxx/> */
xml_tt_T, /* Content for the tag <aaa>xxx</aaa> */
xml_tt_C, /* Comment <!--xx-->*/
xml_tt_ATN, /* Attribute Name <xxx id="">*/
xml_tt_ATV, /* Attribute Value <xxx id="222">*/
xml_tt_CDATA/* <![CDATA[xxxxx]]>*/
}
xml_TokenType;

typedef struct
{
xml_Text text;
xml_TokenType type;
}
xml_Token;


#define MAX_ATT_NUM 50
char tokenValue[MAX_ATT_NUM][MAX_ATT_NUM]={"UNKNOWN","HEAD","NODE_END","NODE_BEGIN","NODE_BEGIN_END","TEXT","COMMENT","ATTRIBUTE_NAME","ATTRIBUTE_VALUE","CDATA"};
char defaultToken[MAX_ATT_NUM]="WRONG_INFO";
 
#define MAX_LINE 1024

char * convertTokenTypeToStr(xml_TokenType type);
int xml_initText(xml_Text *pText, char *s);
int xml_initToken(xml_Token *pToken, xml_Text *pText);
int xml_print(xml_Text *pText, int begin, int end);  //output the content for every parsed element
int xml_println(xml_Text *pText);
char* ltrim(char *s);
char* rtrim(char *s);
int xml_getToken(xml_Text *pText, xml_Token *pToken, int multilineExp, int multilineCDATA);  //parse every element in an xmlText, return value:0--success -1--error 1--multiline explantion 2--multiline CDATA

int layer = 0;  //token the layer every element is in
char multiExpContent[MAX_LINE]={' '};
char multiCDATAContent[MAX_LINE]={' '};

char * convertTokenTypeToStr(xml_TokenType type)
{

	if(type<MAX_ATT_NUM) return tokenValue[type];
	else return defaultToken;
}

int xml_initText(xml_Text *pText, char *s)
{
    pText->p = s;
    pText->len = strlen(s);
    return 0;
}

int xml_initToken(xml_Token *pToken, xml_Text *pText)
{
    pToken->text.p = pText->p;
    pToken->text.len = 0;
    pToken->type = xml_tt_U;
    return 0;
}

int xml_print(xml_Text *pText, int begin, int end)
{
    int i;
    char * temp=pText->p;
    temp = ltrim(pText->p);
    for (i = begin; i < end; i++)
    {
        putchar(temp[i]);
    }
    return 0;
}

int xml_println(xml_Text *pText)
{
    xml_print(pText, 0 , pText->len);
    putchar('\n');
    return 0;
}

char * ltrim(char *s)
{
     char *temp;
     temp = s;
     while((*temp == ' ' || *temp=='\t' )&&temp){*temp++;}
     return temp;
}


int xml_getToken(xml_Text *pText, xml_Token *pToken, int multilineExp, int multilineCDATA)   //return value:0--success -1--error 1--multiline explantion 2--multiline CDATA
{
    char *start = pToken->text.p + pToken->text.len;
    char *p = start;
    char *end = pText->p + pText->len;
    int state = 0;
    int templen = 0;
    if(multilineExp == 1) state = 10;   //1--multiline explantion  0--single line explantion
    if(multilineCDATA == 1) state = 17; //1--multiline CDATA 0--single CDATA
    int j;

    pToken->text.p = p;
    pToken->type = xml_tt_U;
    
    for (; p < end; p++)
    {
        switch(state)
        {
            case 0:
               switch(*p)
               {
                   case '<':
                       state = 1;
                       break;
                   case ' ':
                   	   state = 0;
                   	   break;
                   default:
                       state = 7;
                       break; 
               }
            break;
            case 1:
               switch(*p)
               {
                   case '?':
                       state = 2;
                       break;
                   case '/':
                       state = 4;
                       break;
                   case '!':
                   	   state = 8;
                   	   break;
                   case ' ':
                   	   state = -1;
                   	   break;
                   default:
                       state = 5;
                       break;
               }
            break;
            case 2:
               switch(*p)
               {
                   case '?':
                       state = 3;
                       break;
                   default:
                       state = 2;
                       break;
               }
            break;
            case 3:
               switch(*p)
               {
               	   putchar(*p);
                   case '>':                        /* Head <?xxx?>*/
                       pToken->text.len = p - start + 1;
                       pToken->type = xml_tt_H;
                       //return 1;
                       printf("type=%s;  depth=%d;  ", convertTokenTypeToStr(pToken->type) , layer);
                       printf("%s","content=");
                       templen = pToken->text.len;
                       pToken->text.len -= strlen(pToken->text.p)-strlen(ltrim(pToken->text.p));
                       xml_print(&pToken->text, 0 ,pToken->text.len);
                       printf(";\n\n");
                       pToken->text.p = start + templen;
                       start = pToken->text.p;
                       state = 0;
                       break;
                   default:
                       state = -1;
                       break;
               }
               break;
            case 4:
                switch(*p)
                {
                   case '>':              /* End </xxx> */
                       pToken->text.len = p - start + 1;
                       pToken->type = xml_tt_E;
                       printf("type=%s;  depth=%d;  ", convertTokenTypeToStr(pToken->type) , layer);
                       printf("%s","content=");
                       templen = pToken->text.len;
                       pToken->text.len -= strlen(pToken->text.p)-strlen(ltrim(pToken->text.p));
                       xml_print(&pToken->text, 2 , pToken->text.len-1);
                       printf(";\n\n");
                       pToken->text.p = start + templen;
                       start = pToken->text.p;
                       layer--;
                       state = 0;
                       break;
                       //return 1;
                   case ' ':
                   	   state = -1;
                   	   break;
                   default:
                       state = 4;
                       break;
                }
                break;
            case 5:
                switch(*p)
                {
                   case '>':               /* Begin <xxx> */
                       pToken->text.len = p - start + 1;
                       pToken->type = xml_tt_B;
                       if(pToken->text.len-1 >= 1){
                       	   layer++;
                       	   printf("type=%s;  depth=%d;  ", convertTokenTypeToStr(pToken->type) , layer);
                           printf("%s","content=");
                           templen = pToken->text.len;
                           pToken->text.len -= strlen(pToken->text.p)-strlen(ltrim(pToken->text.p));
                       	   xml_print(&pToken->text , 1 , pToken->text.len-1);
                           printf(";\n\n");
					   }
					   else templen = 1;
                       pToken->text.p = start + templen;
                       start = pToken->text.p;
                       state = 0;
                       break;
                       //return 1;
                   case '/':
                       state = 6;
                       break;
                   case ' ':                 /* Begin <xxx> */
                   	   pToken->text.len = p - start + 1;
                   	   templen = 0;
                       pToken->type = xml_tt_B;
                       if(pToken->text.len-1 >= 1)
                       {
                       	   layer++;
                       	   printf("type=%s;  depth=%d;  ", convertTokenTypeToStr(pToken->type) , layer);
                       	   printf("%s","content=");
                       	   templen = pToken->text.len;
                       	   pToken->text.len -= strlen(pToken->text.p)-strlen(ltrim(pToken->text.p));
                       	   xml_print(&pToken->text , 1 , pToken->text.len-1);
                       	   printf(";\n\n");
					   }
                       pToken->text.p = start + templen;
                       start = pToken->text.p;
                   	   state = 13;
                   	   break;
                   default:
                       state = 5;
                   break;
                }
                break;
            case 6:
                switch(*p)
                {
                   case '>':   /* Begin End <xxx/> */
                       pToken->text.len = p - start + 1;
                       pToken->type = xml_tt_BE;
                       printf("type=%s;  depth=%d;  ", convertTokenTypeToStr(pToken->type) , layer+1);
                       printf("%s","content=");
                       templen = pToken->text.len;
                       pToken->text.len -= strlen(pToken->text.p)-strlen(ltrim(pToken->text.p));
                       xml_print(&pToken->text , 1 , pToken->text.len-2);
                       printf(";\n\n");
                       pToken->text.p = start + templen;
                       start = pToken->text.p;
                       state = 0;
                       break;
                      //return 1;
                   default:
                       state = -1;
                   break;
                } 
                break;
            case 7:
                switch(*p)
                {
                   case '<':     /* Text xxx */
                       p--;
                       pToken->text.len = p - start + 1;
                       pToken->type = xml_tt_T;
                       printf("type=%s;  depth=%d;  ", convertTokenTypeToStr(pToken->type) , layer);
                       printf("%s","content=");
                       templen = pToken->text.len;
                       pToken->text.len -= strlen(pToken->text.p)-strlen(ltrim(pToken->text.p));
                       xml_print(&pToken->text, 0 , pToken->text.len);
                       printf(";\n\n");
                       pToken->text.p = start + templen;
                       start = pToken->text.p;
                       state = 0;
                       break;
                       //return 1;
                   default:
                       state = 7;
                       break;
                }
                break;
            case 8:
            	switch(*p)
            	{
            		case '-':
            			state = 9;
            			break;
            		case '[':
            			if(*(p+1)=='C'&&*(p+2)=='D'&&*(p+3)=='A'&&*(p+4)=='T'&&*(p+5)=='A')
            			{
            				state = 16;
            				p += 5;
            				break;
						}
						else
						{
							state = -1;
							break;
						}
            		default:
            			state = -1;
            			break;
				}
			    break;
			case 9:
				switch(*p)
				{
					case '-':
						state = 10;
						break;
					default:
						state = -1;
						break;
				}
			    break;
			case 10:
				switch(*p)
				{
					case '-':
						state = 11;
						break;
					default:
						state = 10;
						break;
				}
			    break;
			case 11:
				switch(*p)
				{
					case '-':
						state = 12;
						break;
					default:
						state = -1;
						break;
				}
			    break;
			case 12:
				switch(*p)
				{
					case '>':                            /* Comment <!--xx-->*/
					    pToken->text.len = p - start + 1;
                        pToken->type = xml_tt_C;
                        printf("type=%s;  depth=%d;  ", convertTokenTypeToStr(pToken->type) , layer);
                        printf("%s","content=");
                        templen = pToken->text.len;
                        pToken->text.len -= strlen(pToken->text.p)-strlen(ltrim(pToken->text.p));
                        if(multilineExp == 1)
                        {
                        	strcat(multiExpContent,pToken->text.p);
                            printf("%s",ltrim(multiExpContent));
                            printf("\n");
                            memset(multiExpContent, 0 , sizeof(multiExpContent));
						}
						else{
							xml_print(&pToken->text , 0 , pToken->text.len);
                            printf(";\n\n");
						}
						
                        pToken->text.p = start + templen;
                        start = pToken->text.p;
                        state = 0;
						break;
					default:
						state = -1;
						break;
				}
			    break;
			case 13:
				switch(*p)
				{
					case '>':
						state = -1;
						break;
					case '=':                       /*attribute name*/
					    pToken->text.len = p - start + 1;
                        pToken->type = xml_tt_ATN;
                        printf("type=%s;  depth=%d;  ", convertTokenTypeToStr(pToken->type) , layer);
                        printf("%s","content=");
                        templen = pToken->text.len;
                        pToken->text.len -= strlen(pToken->text.p)-strlen(ltrim(pToken->text.p));
                        xml_print(&pToken->text, 0 , pToken->text.len-1);
                        printf(";\n\n");
                        pToken->text.p = start + templen;
                        start = pToken->text.p;
						state = 14;
						break;
					default:
						state = 13;
						break;
				}
			    break;
			case 14:
				switch(*p)
				{
					case '"':                                       
                   	    state = 15;
						break;
					case ' ':
						state = 14;
						break;
					default:
						state = -1;
						break;
				}
			    break;	
			case 15:
				switch(*p)
				{
					case '"':                        /*attribute value*/
						pToken->text.len = p - start + 1;
                        pToken->type = xml_tt_ATV;
                        printf("type=%s;  depth=%d;  ", convertTokenTypeToStr(pToken->type) , layer);
                        printf("%s","content=");
                        templen = pToken->text.len;
                        pToken->text.len -= strlen(pToken->text.p)-strlen(ltrim(pToken->text.p));
                        xml_print(&pToken->text, 1 , pToken->text.len-1);
                        printf(";\n\n");
                        pToken->text.p = start + templen;
                        start = pToken->text.p;
                        state = 5;
						break;
					default:
						state = 15;
						break;
				}
			    break;
			case 16:
				switch(*p)
				{
					case '[':                                       
                   	    state = 17;
						break;
					default:
						state = -1;
						break;
				}
			    break;	
			case 17:
				switch(*p)
				{
					case ']':                                       
                   	    state = 18;
						break;
					default:
						state = 17;
						break;
				}
			    break;	
			case 18:
				switch(*p)
				{
					case ']':                                       
                   	    state = 19;
						break;
					default:
						state = -1;
						break;
				}
			    break;	
			case 19:
				switch(*p)
				{
					case '>':                                       
                   	    pToken->text.len = p - start + 1;
                        pToken->type = xml_tt_CDATA;
                        printf("type=%s;  depth=%d;  ", convertTokenTypeToStr(pToken->type) , layer);
                        printf("%s","content=");
                        templen = pToken->text.len;
                        pToken->text.len -= strlen(pToken->text.p)-strlen(ltrim(pToken->text.p));
                        if(multilineCDATA == 1)
                        {
                        	strcat(multiCDATAContent,pToken->text.p);
                        	for( j = 10;j < strlen(multiCDATAContent) - 3;j++)
                        	{
                        		if(multiCDATAContent[j] == ']' && multiCDATAContent[j+1] == ']') break;
                        		putchar(multiCDATAContent[j]);
							}
                            printf("\n");
                            memset(multiCDATAContent, 0 , sizeof(multiCDATAContent));
						}
						else{
							xml_print(&pToken->text , 9 , pToken->text.len-3);
                            printf(";\n\n");
						}
						
                        pToken->text.p = start + templen;
                        start = pToken->text.p;
                        state = 0;
						break;
					default:
						state = -1;
						break;
				}
			    break;	
				
            default:  
                state = -1;
                break;
        }
    }
    if(state==-1) return -1;
    else if(state == 10)
	{
		strcat(multiExpContent,ltrim(pToken->text.p));
		return 1;
	} 
	else if(state == 17)
	{
		strcat(multiCDATAContent,ltrim(pToken->text.p));
		return 2;
	}
    else return 0;
}

int main()
{
	
    int ret = 0;

    xml_Text xml;
    xml_Token token;
    
    char buf[MAX_LINE];  
    FILE *fp;           
    int len;            
    int multiExp = 0; //0--single line explanation 1-- multiline explanation
    int multiCDATA = 0; //0--single line CDATA 1-- multiline CDATA
    
    printf("Welcome to the XML lexer program! Your file name is test.xml\n\n");
    if((fp = fopen("test.xml","r")) == NULL)
    {
        printf("There are something wrong with the file, we can not load your file. Please check whether it is placed in the right place.");
        exit (1) ;
    }
    printf("The results are listed as follows:\n");
    while(fgets(buf,MAX_LINE,fp) != NULL)
    {
        len = strlen(buf);
        xml_initText(&xml,buf);
        xml_initToken(&token, &xml);
        ret = xml_getToken(&xml, &token, multiExp, multiCDATA);
        if(ret == -1)
        {
        	printf("WARNING: There is something wrong with your xml file, please check its format!");
        	break;
		}
		else if(ret == 1)
		{
        	multiExp = 1;
		}
		else if(ret == 2)
		{
			multiCDATA = 1;
		}
		else{
			multiExp = 0;
			multiCDATA = 0;
		}
    }
    system("pause");
    return 0;
}
