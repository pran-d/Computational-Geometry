#define ADD(head, p) if(head==NULL){\
       head=p;\
       head->next = p;\
       head->prev = p;\
    }\
    else{\
        p->next = head;\
        p->prev = head->prev;\
        head->prev = p;\
        p->prev->next = p;\
    }\

#define DELETE(head, p) if (head){\
    if(head==head->next)\
        head=NULL;\
    else if (p==head)\
        head=head->next;\
    p->next->prev = p->prev;\
    p->prev->next = p->next;\
    free((char *) p); p=NULL;\
}

#define X 0
#define Y 1
#define Z 2

#define VISIBLE true
#define ONHULL true
#define REMOVED true
#define PROCESSED true

