#include <iostream>
#include <cstdlib>
#include <cstring>



class N {

public:
    int _n;
    char _annotation[104];

    N(int n) {
    this->_n = n;
    }

    void setAnnotation(char *annotation) {
        int len = strlen(annotation);
        memcpy(this->_annotation,annotation,len);
    }

    int operator()(N *a, N *b) {
        return a->_n + b->_n;
    }
};


int  main(int argc, const char **argv, const char **envp)
{
    N *v3; 
    N *v4; 
    N *v6;

    if ( argc <= 1 )
    exit(1);
    v3 = new N(5);
    v6 = v3;
    v4 = new N(6);
    v6->setAnnotation((char *)argv[1]);
    return (**(int (***)(N *, N *))v4)(v4, v6);
}