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
#define BLOCK_SIZE(header) *((size_t *)header) & 0x1
#define IS_ALLOC(header) *((size_t *)header) & ~0xfL

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    return 0;
}


/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */

typedef struct _node{
    void* blockptr;
	struct _node* next;
}node;

node* seg_list[1100];

void mov_to_next(node *pre_node, node *cur_node){
    if(pre_node = cur_node){
        cur_node = cur_node->next;
    }else{
        pre_node = cur_node;
        cur_node = cur_node->next;
    }
}

void del_node(node* pre_node, node *cur_node){
    if(pre_node = cur_node){
        pre_node = pre_node->next;
    }else{
        pre_node->next = pre_node->next->next;
    }
}

void find_and_del(size_t block_size, void *header){
    node *pre_node, *cur_node;

    pre_node = seg_list[(int)block_size];
    cur_node = seg_list[(int)block_size];
    while(cur_node != NULL){

        if(cur_node == header){
            del_node(pre_node, cur_node);
            break;
        }
        mov_to_next(pre_node, cur_node);
    }

}

void add_node(node* new_node, node* cur_node){
    node* next_node = cur_node->next;
    new_node->next = next_node ;
    cur_node->next = new_node ;
}

void add_remain_to_list(void *header, size_t pre_size, size_t block_size){
    
    node *newnode;

    *((size_t *)header) = block_size;
    *((size_t *)((char *)header + block_size)) = block_size;
    *((size_t *)((char *)header - SIZE_T_SIZE)) = pre_size | 0x1;

    newnode->blockptr = header;
    add_node(newnode, seg_list[(int)block_size]);
}


int two_power(n){
    int j;
    int ret = 1;
    for(j = 0; j < n; j++){
        ret *= 2;
    }
    return ret;
}

void *seg_alloc(size){
    int i, n;
    void* header;
    node *pre_node, *cur_node;

    // 1025 이상의 수는 로그 스케일링
    if(size >= 1025){
        for(i = 0; i < 100; i++){
            if(size > two_power(i + 10) && size <= two_power(i + 11)){
                size = 1025 + i;
                break;
            }
        }
    }
    
    // 주어진 size 이상의 미할당블록 검색 및 할당 그리고 분할

    for(n = size; n < 1100; n++){
        if(seg_list[n] != NULL){
            pre_node = seg_list[n] ;
            cur_node = seg_list[n] ;
            while(cur_node != NULL){
                header = cur_node->blockptr;

                if(IS_ALLOC(header) == 0 ){
                    *((size_t *)header) = BLOCK_SIZE(header) | 0x1;

                    add_remain_to_list((void *)((char *)header + size), (size_t)size, BLOCK_SIZE(header) - (size_t)size);
                    del_node(pre_node, cur_node);

                    return (void *)((char *)header + SIZE_T_SIZE);

                }else{
                    //free 하는 과정에서 버려진 free node를 발견
                    del_node(pre_node, cur_node);
                }

                mov_to_next(pre_node, cur_node);
                
            }
        }
    }
}

void *mm_malloc(size_t size)
{
    // header, footer를 포함해야하므로 SIZE_T_SIZE의 두배를 추가로 할당
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
    if (p == (void *)-1)
	return NULL;
    else {
        // header, footer 에 size, 할당여부 기록
        *(size_t *)p = (size_t)newsize | 0x1;
        *(size_t *)((char *)p + SIZE_T_SIZE + newsize) = (size_t)newsize | 0x1 ;
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
    void *footer = (void *)((char *)ptr + block_size);

    void *pre_footer = (void *)((char *)header - SIZE_T_SIZE);
    block_size = BLOCK_SIZE(pre_footer);
    void *pre_header = (void *)((char *)pre_footer - block_size);

    void *next_header = (void *)((char *)footer + SIZE_T_SIZE);
    block_size = BLOCK_SIZE(next_header);
    void *next_footer = (void *)((char *)next_header + block_size);

    node *newnode, *pre_node, *cur_node;
    
    //case1: prev = alloc, next = alloc
    if(IS_ALLOC(pre_footer) && IS_ALLOC(next_header)){
        *(size_t *)header = *(size_t *)header & ~0xfL;
        *(size_t *)footer = *(size_t *)header & ~0xfL;
        block_size = BLOCK_SIZE(header);

        newnode->blockptr = header;
        add_node(newnode, seg_list[(int)block_size]);

    }else if( IS_ALLOC(pre_footer) && !IS_ALLOC(next_header) ){
        block_size = BLOCK_SIZE(header) + BLOCK_SIZE(next_header);
        *(size_t *)header = block_size;
        *(size_t *)next_footer = block_size;
        
        newnode->blockptr = header;
        add_node(newnode, seg_list[(int)block_size]);

        //기존의 free 노드 비활성화
        *(size_t *)next_header = *(size_t *)next_header | 0x1;

    }else if(IS_ALLOC(pre_footer) == 0 && IS_ALLOC(next_header)){
        block_size = BLOCK_SIZE(pre_header) + BLOCK_SIZE(header);
        *(size_t *)footer = block_size;
        *(size_t *)pre_header = block_size;

        newnode->blockptr = pre_header;
        add_node(newnode, seg_list[(int)block_size]);

        //앞에있는 free노드는 겹치기때문에 찾아서 제거
        block_size = BLOCK_SIZE(pre_header);
        find_and_del(block_size, pre_header);


    }else{
        block_size = BLOCK_SIZE(pre_header) + BLOCK_SIZE(header) + BLOCK_SIZE(next_header);
        *(size_t *)pre_header = block_size;
        *(size_t *)next_footer = block_size;

        newnode->blockptr = pre_header;
        add_node(newnode, seg_list[(int)block_size]);

        //뒤에 있는 free 노드 비활성화
        *(size_t *)next_header = *(size_t *)next_header | 0x1;
        //앞에 있는 free 노드 검색 및 제거
        block_size = BLOCK_SIZE(pre_header);
        find_and_del(block_size, pre_header);
    }    

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














