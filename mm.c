/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "legeno",
    /* First member's full name */
    "leejonghyeong",
    /* First member's email address */
    "leejonghyeong726@gmail.com",
    /* Second member's full name (leave blank if none) */
    "none",
    /* Second member's email address (leave blank if none) */
    "none"
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

//define size and alloc info
#define BLOCK_SIZE(header) (*((size_t *)header) & ~0x7)
#define IS_ALLOC(header) (*((size_t *)header) & 0x1)


void** seg_list[1100] = {NULL};
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    int i;
    //
    //free list 초기화
    for(i=0; i<1100; i++)
        seg_list[i] = NULL;
    

    return 0;
}


/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */

int two_power(n){
    int j;
    int ret = 1;
    for(j = 0; j < n; j++){
        ret *= 2;
    }
    return ret;
}

size_t log_scale(size_t blocksize){
    // 1025 이상의 수는 로그 스케일링
    int i;
    int log_size = (int) blocksize;

    if(log_size >= 1025)
        for(i = 0; i < 100; i++){
            if(log_size > two_power(i + 10) && log_size <= two_power(i + 11)){
                log_size = 1025 + i;
                break;
            }
        }
    return log_size;
}


/*
 *  Add free block(header) into the cycle of pointers.
 *  each pointer indicates prev or next header(which is also a pointer).
 *  So, these pointers are in fact double pointers.
 */
void add_free_block(size_t block_size, size_t* header){
    size_t log_size = log_scale(block_size);
    if (seg_list[(int)log_size] == NULL){
        seg_list[(int)log_size] = (void *) &header;
        *(seg_list[(int)log_size] + SIZE_T_SIZE) = NULL;
        *(seg_list[(int)log_size] + 2 * SIZE_T_SIZE) = NULL;
    }else{
        void** first_block = seg_list[(int)log_size];
        void** last_block = (void *)((char *)(*first_block) + SIZE_T_SIZE);
        *(void **)((char *)last_block + 2 * SIZE_T_SIZE) = &header;
        *(void **)((char *)header + SIZE_T_SIZE) = &last_block;
        *(void **)((char *)header + 2 * SIZE_T_SIZE) = &first_block;
        *(void **)((char *)first_block + SIZE_T_SIZE) = &header;
    }
}
/*
 *  Remove free block from the cycle of pointers.
 *  "To remove" means to connect its predecessor and successor.
 *  If this free block is the only free block in the cycle for certain block size,
 *  place it by null pointer.
 */
void remove_free_block(size_t block_size, size_t* header){
    size_t log_size = log_scale(block_size);
    void** pred_block = (void *) ((char *)header + SIZE_T_SIZE);
    void** succ_block = (void *) ((char *)header + 2 * SIZE_T_SIZE);

    if (*pred_block == NULL && *succ_block == NULL)
        seg_list[(int) log_size] = NULL;
    else{
        // *pre_block = pre_header 주소
        // *succ_block = next_header 주소
        *((char *)*pred_block + 2 * SIZE_T_SIZE) = *succ_block;
        *((char *)*succ_block + SIZE_T_SIZE) = *pred_block;
    }
}


void *seg_alloc(size_t size){
    int n;
    void** last_block = NULL;
    int log_size = (int) log_scale(size);

    // Use last free block in the cycle corresponding to given size.
    for(n = log_size; n < 1100; n++){
        if(seg_list[n] != NULL){
            // predecessor of the first block is the last block
            *last_block = *(seg_list[n]) + SIZE_T_SIZE;

            // remove last free block
            remove_free_block(BLOCK_SIZE(*last_block), *last_block);
            }
    }

    return (void *) -1 ;
}

void *mm_malloc(size_t size)
{
    // header, footer를 포함해야하므로 2 * SIZE_T_SIZE 추가 할당
    int newsize = ALIGN(size + 2 * SIZE_T_SIZE);
    void* ptr;

    // newsize에 해당하는 '할당되지 않은' 블록을 검색
    // 검색방법은 seg list
    // 해당되는 블록이 있으면 할당하고 블록 분할.
    // 없다면 힙 추가 요청
    ptr = seg_alloc(newsize);
    if(ptr != (void *) - 1){
        return ptr;
    }

    void *p = mem_sbrk(newsize);
    if (p == (void *)-1){
	    return NULL;
    }else{
        // header, footer 에 size, 할당여부 기록    
        *(size_t *)p = (size_t)newsize | 0x1;
        *(size_t *)((char *)p + newsize - SIZE_T_SIZE) = (size_t)newsize | 0x1 ;
    
        printf("SIZE_T_SIZE: %d\n", SIZE_T_SIZE);
        printf("newsize: %d\n", newsize);
        printf("header: %#x\n", p);
        printf("footer:%#x\n", ((char *)p - SIZE_T_SIZE + newsize));
        printf("%zu\n",IS_ALLOC(p));
    
        return (void *)((char *)p + SIZE_T_SIZE);
    }

}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    // header에 비어있는 상태 기록
    // 블록 합치기 (4가지 경우)
    // seg_list에 추가
    size_t block_size;
    void *header = (void *)((char *)ptr - SIZE_T_SIZE);
    block_size = BLOCK_SIZE(header);
    //printf("\n current block size: %zd\n", block_size);
    void *footer = (void *)((char *)ptr + block_size - 2 * SIZE_T_SIZE);

    void *pre_footer = (void *)((char *)header - SIZE_T_SIZE);
    block_size = BLOCK_SIZE(pre_footer);
    //printf("\n pre block size: %zd\n", block_size);
    void *pre_header = (void *)((char *)pre_footer - block_size + SIZE_T_SIZE);

    void *next_header = (void *)((char *)footer + SIZE_T_SIZE);
    block_size = BLOCK_SIZE(next_header);
    //printf("\n next block size: %zd\n", block_size);
    void *next_footer = (void *)((char *)next_header + block_size - SIZE_T_SIZE);


    printf("\n\n");
    printf("heap checking:\n");
	printf("last heap byte: %#x\n",mem_heap_hi());
	printf("first heap byte: %#x\n",mem_heap_lo());
	printf("heap size: %d\n",mem_heapsize());
    printf("\n\n");
    printf("pre:\n");
    printf("pre_header: %#x\n", pre_header);
    printf("pre_footer: %#x\n", pre_footer);
    printf("\n pre_header \n");
    printf("BLOCKSIZE_PRE:%d\n", BLOCK_SIZE(pre_header));
    printf("Is_ALlOC_PRE:%d\n", IS_ALLOC(pre_header));
    printf("\n pre_footer \n");
    printf("BLOCKSIZE_PRE:%d\n", BLOCK_SIZE(pre_footer));
    printf("Is_ALlOC_PRE:%d\n", IS_ALLOC(pre_footer));

    printf("\n\n");
    printf("current:\n");
    printf("header: %#x\n", header);
    printf("footer: %#x\n", footer);
    printf("\nheader \n");
    printf("BLOCKSIZE:%d\n", BLOCK_SIZE(header));
    printf("Is_ALlOC:%d\n", IS_ALLOC(header));
    printf("\nfooter \n");
    printf("BLOCKSIZE:%d\n", BLOCK_SIZE(footer));
    printf("Is_ALlOC:%d\n", IS_ALLOC(footer));

    printf("\n\n");
    printf("next:\n");
    printf("next_header: %#x\n", next_header);
    printf("next_footer: %#x\n", next_footer);
    printf("\n next_header \n");
    printf("BLOCKSIZE_next:%d\n", BLOCK_SIZE(next_header));
    printf("Is_ALlOC_next:%d\n", IS_ALLOC(next_header));
    printf("\n next_footer \n");
    printf("BLOCKSIZE:%d\n", BLOCK_SIZE(next_footer));
    printf("Is_ALlOC:%d\n", IS_ALLOC(next_footer));
  
    //case1: prev = alloc, next = alloc
    if(IS_ALLOC(pre_footer) && IS_ALLOC(next_header)){
        *(size_t *)header = BLOCK_SIZE(header);
        *(size_t *)footer = BLOCK_SIZE(header);
        block_size = BLOCK_SIZE(header);
        
        // free list에 연결
        add_free_block(block_size, header);

    }else if( IS_ALLOC(pre_footer) && (IS_ALLOC(next_header) == 0) ){
        block_size = BLOCK_SIZE(header) + BLOCK_SIZE(next_header);
        *(size_t *)header = block_size;
        *(size_t *)next_footer = block_size;
        
        //free list에 연결
        add_free_block(block_size, header);

        //remove existing free block
        remove_free_block(BLOCK_SIZE(next_header), next_header);

    }else if((IS_ALLOC(pre_footer) == 0) && IS_ALLOC(next_header)){
        block_size = BLOCK_SIZE(pre_header) + BLOCK_SIZE(header);
        *(size_t *)footer = block_size;
        *(size_t *)pre_header = block_size;

        //free list에 연결
        add_free_block(block_size, pre_header);

        //remove existing free block
        remove_free_block(BLOCK_SIZE(pre_header), pre_header);

    }else{
        block_size = BLOCK_SIZE(pre_header) + BLOCK_SIZE(header) + BLOCK_SIZE(next_header);
        *(size_t *)pre_header = block_size;
        *(size_t *)next_footer = block_size;

        //free list에 연결
        add_free_block(block_size, pre_header);

        //remove existing free blocks
        remove_free_block(BLOCK_SIZE(pre_header), pre_header);
        remove_free_block(BLOCK_SIZE(next_header), next_header);
    }    
    printf("hello");
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














