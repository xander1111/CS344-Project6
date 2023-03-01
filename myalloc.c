#include <stdio.h>
#include <sys/mman.h>


#define ALIGNMENT 16
#define GET_PAD(x) ((ALIGNMENT - 1) - (((x) - 1) & (ALIGNMENT - 1)))
#define PADDED_SIZE(x) ((x) + GET_PAD(x))

#define PTR_OFFSET(p, offset) ((void*)((char *)(p) + (offset)))

#define PADDED_BLOCK PADDED_SIZE(sizeof(struct block))


struct block {
    struct block* next;
    struct block* prev;
    int size;
    short in_use;
};

struct block* head;


void split_space(struct block* to_split, int size) {
    if (to_split->size > size + PADDED_BLOCK) {
        struct block* new_block = PTR_OFFSET(to_split, size + PADDED_BLOCK);
        new_block->next = to_split->next;
        new_block->prev = to_split;
        new_block->size = to_split->size - size - PADDED_BLOCK;
        new_block->in_use = 0;

        if (to_split->next) to_split->next->prev = new_block;

        to_split->next = new_block;
    }
}


void* find_space(int size) {
    size = PADDED_SIZE(size);

    struct block* current = head;

    do {
        if (!current->in_use && current->size >= size) {
            split_space(current, size);

            current->in_use = 1;
            current->size = size;

            return PTR_OFFSET(current, PADDED_BLOCK);
        } else {
            current = current->next;
        }
    } while (current != NULL);

    return NULL;
}


void merge_blocks(struct block* to_merge) {
    if (to_merge && !to_merge->in_use && to_merge->next && !to_merge->next->in_use) {
        to_merge->size = to_merge->size + to_merge->next->size + PADDED_BLOCK;
        to_merge->next = to_merge->next->next;
        
        if (to_merge->next) to_merge->next->prev = to_merge;
    }
}


void* myalloc(int size) {
    if (head == NULL) {
        head = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
        head->next = NULL;
        head->prev = NULL;
        head->size = 1024 - PADDED_BLOCK;
        head->in_use = 0;
    }

    return find_space(size);
}


void myfree(void* to_free) {
    struct block* block_to_free = PTR_OFFSET(to_free, -1 * PADDED_BLOCK);
    block_to_free->in_use = 0;
    merge_blocks(block_to_free);
    merge_blocks(block_to_free->prev);
}


void print_data(void) {
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


int main(void) {
    printf("block byte size: %d\n", sizeof(struct block));
    printf("padded block byte size: %d\n", PADDED_BLOCK);

    void* test;
    void* test2;

    print_data();
    test = myalloc(60);
    print_data();
    test2 = myalloc(60);
    print_data();
    myfree(test);
    print_data();
    myfree(test2);
    print_data();
}

