void mem_init();
void pageFrame_init();
char *mem_get_value(char *var);
int mem_set_value(char *var_in, char *value_in, int n);
void mem_reset_codeloading();

void backingStore_init();
void backingStore_remove();
void mem_reset_varStore();
void runIndex(int frameIndex, int lineIndex);
void mem_reset_index(int index);
char* evictLeastRecentlyUsed(int* evictedPageFrame);
void printPageFault(int frame);
void updateRecentlyUsed(int frame);
