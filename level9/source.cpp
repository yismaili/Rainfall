class N {
private:
    int number;
    char *annotation;

public:
    N(int n) {
        number = n;
    }
    
    virtual void operator+(N& other) = 0;  // pure virtual function
    
    void setAnnotation(char *str) {
        memcpy(annotation, str, strlen(str));
    }
};

int main(int argc, char *argv[]) {
    if(argc <= 1) {
        _exit(1);
    }
    
    // (108) bytes
    N* n1 = new N(5);
    N* n2 = new N(6);
    
    N* a = n1;
    N* b = n2;
    
    a->setAnnotation(argv[1]);
    
    // call virtual function (operator+)
    (*b) + (*a);
    
    return 0;
}