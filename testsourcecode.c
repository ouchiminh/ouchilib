#include "circularly-linked-list.h"
#include <stdlib.h>

struct circularly_node* create() {
    struct circularly_node* first = (struct circularly_node*)malloc(sizeof(struct circularly_node));

    if (first == NULL) {
        return NULL;
    }
    
    first->next_ = first;
    first->prev_ = first;
    return first;
}

struct circularly_node* insert(struct circularly_node* pos, DATA_TYPE val) {
    struct circularly_node* newone = (struct circularly_node*)malloc(sizeof(struct circularly_node));
    if (newone == NULL)
        return NULL;

    if (pos == NULL) {
        newone->next_ = newone;
        newone->prev_ = newone;
        newone->val_ = val;
        return newone;
    }

    newone->next_ = pos;
    newone->prev_ = pos->prev_;
    newone->val_ = val;

    pos->prev_->next_ = newone;
    pos->prev_ = newone;
    return newone;
}

struct circularly_node* erase(struct circularly_node* pos) {
	if(pos == NULL) return NULL;
	struct circularly_node* next = pos->next_;
	struct circularly_node* prev = pos->prev_;

	free(pos);
	next->prev_ = prev;
	prev->next_ = next;
	return next;
}

struct circularly_node* forward(struct circularly_node* pos) {
    if (pos == NULL) return NULL;
	return pos->next_;
}
