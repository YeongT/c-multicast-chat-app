#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

void pprint(char str[]) 
{
    int null_c = 0;
    for (int i = 0; i < sizeof(str); i++)
    {
        if (str[i] != '\0')
        {
            if(null_c > 0){
                printf("(%dnull)", null_c);
                null_c = 0;
            }
            printf("%c", str[i]);
        }
        else
        {
            null_c += 1;
            
        }
    }
}



int main()
{
    char s[50] = "aaa\0\0\0\0ssfsf", t[50];

    pprint(s);
    printf("\n%d null character detected in %s\n",printWithoutNull(s,t), t);
}
