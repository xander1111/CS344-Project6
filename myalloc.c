#include <stdio.h>
#include <sys/mman.h>


#define ALIGNMENT 16
#define GET_PAD(x) ((ALIGNMENT - 1) - (((x) - 1) & (ALIGNMENT - 1)))
#define PADDED_SIZE(x) ((x) + GET_PAD(x))

#define PTR_OFFSET(p, offset) ((void*)((char *)(p) + (offset)))

#define PADDED_BLOCK PADDED_SIZE(sizeof(struct block))


struct block {
    struct block* next;
    int size;
    int in_use;
};

struct block* head;


void* myalloc(int size) {
    if (head == NULL) {
        head = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
        head->next = NULL;
        head->size = 1024 - PADDED_BLOCK;
        head->in_use = 0;
    }

    struct block* current = head;

    do {
        if (!current->in_use && current->size >= size) {
            current->in_use = 1;

            return PTR_OFFSET(current, PADDED_BLOCK);
        }
    } while (current->next != NULL);

    return NULL;
}


void print_data(void)
{
    struct block *b = head;

    if (b == NULL) {
        printf("[empty]\n");
        return;
    }

    while (b != NULL) {
        // Uncomment the following line if you want to see the pointer values
        //printf("[%p:%d,%s]", b, b->size, b->in_use? "used": "free");
        printf("[%d,%s]", b->size, b->in_use? "used": "free");
        if (b->next != NULL) {
            printf(" -> ");
        }

        b = b->next;
    }

    printf("\n");
}


int main(void)
{
    void* test;

    print_data();
    test = myalloc(64);
    print_data();
    test = myalloc(64);
    printf("%p\n", test);
}

