#include <stdio.h>
#include <sys/mman.h>


#define ALIGNMENT 16
#define GET_PAD(x) ((ALIGNMENT - 1) - (((x) - 1) & (ALIGNMENT - 1)))
#define PADDED_SIZE(x) ((x) + GET_PAD(x))

#define PTR_OFFSET(p, offset) ((void*)((char *)(p) + (offset)))


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
        head->size = 1024 - PADDED_SIZE(sizeof(struct block));
        head->in_use = 0;
    }

    struct block* current = head;

    do {
        if (!current->in_use && current->size >= size) {
            current->in_use = 1;

            return PTR_OFFSET(current, PADDED_SIZE(sizeof(struct block)));
        }
    } while (current->next != NULL);

    return NULL;
}


int main(void)
{
    int* test = myalloc(64);
    printf("%p\n", test);
}

