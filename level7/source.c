#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct Node {
    int id;
    char *str;
};

char c[68];

void m() {
    time_t t;
    time(&t);
    printf("%s - %d\n", c, (int)t);
}

int main(int argc, char *argv[]) {
    struct Node *node1, *node2;
    
    node1 = malloc(sizeof(struct Node));
    node1->id = 1;
    node1->str = malloc(8);
    
    node2 = malloc(sizeof(struct Node));
    node2->id = 2;
    node2->str = malloc(8);
    

    strcpy(node1->str, argv[1]);
    
    strcpy(node2->str, argv[2]);
    
    FILE *fp = fopen("/home/user/level8/.pass", "r");
    fgets(c, 68, fp);
    
    puts("~~");
    
    return 0;
}