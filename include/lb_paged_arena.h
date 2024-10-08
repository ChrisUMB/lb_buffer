// ReSharper disable CppNonInlineFunctionDefinitionInHeaderFile
#ifndef LB_PAGED_ARENA_H
#define LB_PAGED_ARENA_H

#ifdef __cplusplus
#include <cstddef>
#include <cstdlib>
extern "C" {
#else
#include <stddef.h>
#include <stdlib.h>
#endif

typedef struct LB_PagedArenaPage LB_PagedArenaPage;
typedef struct LB_PagedArena LB_PagedArena;

LB_PagedArena * lbPagedArenaNew(size_t default_page_capacity);
void lbPagedArenaFree(LB_PagedArena *arena);
void lbPagedArenaClear(LB_PagedArena *arena);
void* lbPagedArenaAlloc(LB_PagedArena *arena, size_t size);

#ifdef __cplusplus
}
#endif

#endif //LB_PAGED_ARENA_H

#ifdef LB_PAGED_ARENA_IMPLEMENTATION
#ifdef __cplusplus
extern "C" {
#endif

typedef struct LB_PagedArenaPage {
    LB_PagedArenaPage *next;
    LB_PagedArenaPage *prev;
    size_t capacity;
    size_t length;
    void *data;
} LB_PagedArenaPage;

typedef struct LB_PagedArena {
    LB_PagedArenaPage *head;
    LB_PagedArenaPage *tail;
    size_t default_page_capacity;
} LB_PagedArena;

static LB_PagedArenaPage *lbPagedArenaPageNew(size_t capacity) {
    void* data = malloc(sizeof(LB_PagedArenaPage) + capacity);
    LB_PagedArenaPage *page = data;
    if(page == NULL) {
        return NULL;
    }

    page->data = data + sizeof(LB_PagedArenaPage);
    page->capacity = capacity;
    page->length = 0;
    page->next = NULL;
    page->prev = NULL;
    return page;
}

static void lbPagedArenaPageFree(LB_PagedArenaPage *page) {
    free(page);
}

LB_PagedArena * lbPagedArenaNew(size_t default_page_capacity) {
    if(default_page_capacity == 0) {
        return NULL;
    }

    LB_PagedArenaPage *page = lbPagedArenaPageNew(default_page_capacity);
    if(page == NULL) {
        return NULL;
    }

    LB_PagedArena *arena = (LB_PagedArena *)malloc(sizeof(LB_PagedArena));
    if(arena == NULL) {
        free(page);
        return NULL;
    }

    arena->default_page_capacity = default_page_capacity;
    arena->head = page;
    arena->tail = page;
    return arena;
}

void lbPagedArenaFree(LB_PagedArena *arena) {
    LB_PagedArenaPage *page = arena->head;
    while(page != NULL) {
        LB_PagedArenaPage *next = page->next;
        lbPagedArenaPageFree(page);
        page = next;
    }

    free(arena);
}

void lbPagedArenaClear(LB_PagedArena *arena) {
    LB_PagedArenaPage *page = arena->head;
    while(page != NULL) {
        page->length = 0;
        page = page->next;
    }
}

void * lbPagedArenaAlloc(LB_PagedArena *arena, const size_t size) {
    // Step 1: Find a page with enough space
    LB_PagedArenaPage *page = arena->head;
    while(page != NULL) {
        if(page->length + size <= page->capacity) {
            break;
        }
        page = page->next;
    }

    if(page == NULL) {
        // Step 1.5:
        // If no page with enough space was found create a new
        // page with a multiple of the default page capacity that
        // is greater than the requested size.
        size_t new_capacity = arena->default_page_capacity;
        while(new_capacity < size) {
            new_capacity *= 2;
        }

        page = lbPagedArenaPageNew(new_capacity);
        if(page == NULL) {
            return NULL;
        }

        page->prev = arena->tail;
        arena->tail->next = page;
        arena->tail = page;
    }

    // Step 2: Allocate the memory in the page
    void *ptr = page->data + page->length;
    page->length += size;
    return ptr;
}

#ifdef __cplusplus
}
#endif

#endif //LB_PAGED_ARENA_IMPLEMENTATION
