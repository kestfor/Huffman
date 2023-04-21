#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>


#define EMPTY (-2)
#define NYT (-3)
#define END (-4)
#define MAX_LEN 256
#define MAX_AMOUNT_NODES ((MAX_LEN + 2) * 2 - 1)
#define LEN_BYTE 8
#define MAX_NODE_SIZE SHRT_MAX
#define BUFF_SIZE 10000


typedef struct {
    unsigned char *string;
    FILE *stream;
    int capacity;
    int size;
} Buffer;


Buffer *buffer_init(FILE *stream, int len) {
    Buffer *new = malloc(sizeof(Buffer));
    new->string = malloc(sizeof(unsigned char) * BUFF_SIZE);
    new->stream = stream;
    new->capacity = len;
    new->size = 0;
    return new;
}


void clear_buffer(Buffer *buff) {
    free(buff->string);
    free(buff);
}


int get_char(Buffer *buff) {
    if (buff->size == buff->capacity || buff->capacity == 0) {
        buff->capacity = (short int) fread(buff->string, sizeof(unsigned char), BUFF_SIZE, buff->stream);
        buff->size = 0;
    }
    if (buff->size < buff->capacity) {
        buff->size++;
        return buff->string[buff->size - 1];
    } else {
        return -1;
    }
}


void put_char(Buffer *buff, unsigned char c) {
    if (buff->size == buff->capacity) {
        fwrite(buff->string, sizeof(unsigned char), buff->size, buff->stream);
        buff->size = 0;
        put_char(buff, c);
    } else {
        buff->string[buff->size] = c;
        buff->size++;
    }
}


void print_buffer(Buffer *buff) {
    fwrite(buff->string, sizeof(unsigned char), buff->size, buff->stream);
    buff->size = 0;
}


typedef struct Byte {
    int count_filled;
    char bits[LEN_BYTE + 1];
} Byte;


typedef struct TreeNode {
    int letter;
    int amount;
    struct TreeNode *right;
    struct TreeNode *left;
    struct TreeNode *parent;
} TreeNode;


typedef struct Queue {
    TreeNode *array[MAX_AMOUNT_NODES];
    int first;
    int last;
} Queue;


enum mode {
    zip = (int) 'c',
    unzip = (int) 'd',
};

TreeNode *extract(Queue *my_queue) {
    if (my_queue->last - my_queue->first > 0) {
        TreeNode *node = my_queue->array[my_queue->first % MAX_AMOUNT_NODES];
        my_queue->first++;
        return node;
    } else {
        return NULL;
    }
}


void add(Queue *my_queue, TreeNode *node) {
    my_queue->array[my_queue->last % MAX_AMOUNT_NODES] = node;
    my_queue->last++;
}


bool is_empty(Queue *my_queue) {
    return my_queue->last - my_queue->first == 0;
}


int min(int first, int second) {
    return (first < second) ? first : second;
}

void swap(void *el1, void *el2, int size) {
    for (int i = 0; i < size; i++) {
        char buf = *((char *) el1 + i);
        *((char *) el1 + i) = *((char *) el2 + i);
        *((char *) el2 + i) = buf;
    }
}


//обход дерева в ширину, перестройка массива при свапе листьев с детьми
void rebuild_array(TreeNode **array_form, int ind_start) {
    int curr_ind = 0;
    Queue my_queue = {{0}, 0, 0};
    add(&my_queue, array_form[0]);
    while (!is_empty(&my_queue)) {
        TreeNode *node = extract(&my_queue);
        if (curr_ind >= ind_start) {
            array_form[curr_ind] = node;
        }
        if (node->right != NULL) {
            add(&my_queue, node->right);
        }
        if (node->left != NULL) {
            add(&my_queue, node->left);
        }
        curr_ind++;
    }
}

bool has_kids(TreeNode *n) {
    return n->left != NULL || n->right != NULL;
}


void swap_nodes(TreeNode **array_form, int ind_first_error, int ind_second_error) {
    TreeNode *node1 = array_form[ind_first_error];
    TreeNode *node2 = array_form[ind_second_error];
    if (node1->parent == node2->parent) {
        TreeNode *left = (node1->parent->left == node1) ? node1 : node2;
        TreeNode *right = (node1->parent->right == node1) ? node1 : node2;
        TreeNode *parent = node1->parent;
        parent->left = right;
        parent->right = left;
    } else {
        TreeNode *parent1 = node1->parent;
        TreeNode *parent2 = node2->parent;
        if (parent1->left == node1) {
            parent1->left = node2;
            if (parent2->left == node2) {
                parent2->left = node1;
            } else {
                parent2->right = node1;
            }
        } else {
            parent1->right = node2;
            if (parent2->left == node2) {
                parent2->left = node1;
            } else {
                parent2->right = node1;
            }
        }
        node1->parent = parent2;
        node2->parent = parent1;
    }
    if (has_kids(node1) || has_kids(node2)) {
        int ind_start = min(ind_first_error, ind_second_error);
        rebuild_array(array_form, ind_start);
    } else {
        swap(&array_form[ind_first_error], &array_form[ind_second_error], sizeof(TreeNode **));
    }
}


TreeNode *node_init(int letter, int amount, TreeNode *left, TreeNode *right, TreeNode *parent) {
    TreeNode *new_node = malloc(sizeof(TreeNode));
    *new_node = (TreeNode) {letter, amount, left, right, parent};
    return new_node;
}


int bin_to_int(char bin[]) {
    int result = 0;
    int len = (int) strlen(bin);
    int power = 1;
    for (int i = len - 1; i >= 0; i--) {
        result += power * (bin[i] - '0');
        power *= 2;
    }
    return result;
}


void int_to_bin(char *code, int symbol) {
    int len = 0;
    while (symbol) {
        code[LEN_BYTE - len - 1] = (char) (symbol % 2 + '0');
        symbol /= 2;
        len++;
    }
    for (int i = 0; i < LEN_BYTE - len; i++) {
        code[i] = '0';
    }
}


//байт на вывод
void update_byte(Byte *output_byte, char code[], Buffer *buff) {
    int len_code = (short int) strlen(code);
    for (int i = 0; i < len_code; i++) {
        output_byte->bits[output_byte->count_filled] = code[i];
        output_byte->count_filled++;
        if (output_byte->count_filled == LEN_BYTE) {
            put_char(buff, bin_to_int(output_byte->bits));
            output_byte->count_filled = 0;
        }
    }
}


//масштабирование дерева от листьев без детей и перерасчет родителей
int scale_tree(TreeNode *node) {
    int left_amount = 0;
    int right_amount = 0;
    if (node->left != NULL) {
        left_amount = scale_tree(node->left);
    }
    if (node->right != NULL) {
        right_amount = scale_tree(node->right);
    }
    if (node->left == NULL && node->right == NULL) {
        node->amount = node->amount / 2;
        return node->amount;
    } else {
        node->amount = left_amount + right_amount;
        return node->amount;
    }
}


void update_parents(TreeNode *node) {
    while (node->parent != NULL) {
        TreeNode *parent = node->parent;
        if (parent->right == node) {
            parent->amount = parent->left->amount + node->amount;
        } else {
            parent->amount = parent->right->amount + node->amount;
        }
        node = parent;
    }
}


//перерасчет родителей свапнутых листьев
void recalculate_tree(TreeNode *first_node, TreeNode *second_node) {
    update_parents(first_node);
    update_parents(second_node);
}


void get_code(TreeNode *node, char *dest) {
    int len_code = 0;
    while (node->parent != NULL) {
        dest[len_code] = (node->parent->left == node) ? '0' : '1';
        len_code++;
        node = node->parent;
    }
    for (int i = 0; i < len_code / 2; i++) {
        swap(&dest[i], &dest[len_code - 1 - i], sizeof(char));
    }
}


bool is_sorted(TreeNode *array_form[], int len_array, int *first_ind, int *second_ind) {
    int ind_viol1;
    int ind_viol2;
    bool is_sorted = true;
    for (int i = len_array - 1; i > 0; i--) {
        if (array_form[i]->amount > array_form[i - 1]->amount) {
            is_sorted = false;
            ind_viol1 = i;
            while (ind_viol1 < len_array - 1 && array_form[ind_viol1]->amount == array_form[ind_viol1 + 1]->amount) {
                ind_viol1++;
            }
            ind_viol2 = i - 1;
            for (int j = ind_viol2; j >= 0; j--) {
                if (array_form[j]->amount < array_form[ind_viol1]->amount) {
                    ind_viol2 = j;
                }
            }
            break;
        }
    }
    if (!is_sorted) {
        *first_ind = ind_viol1;
        *second_ind = ind_viol2;
        return false;
    } else {
        return true;
    }
}


//проверка на дерево хаффмана, починка дерева при необходимости
void repair_tree(TreeNode *array_form[], int len_array) {
    int ind_first_error;
    int ind_second_error;
    bool is_huffman_tree = is_sorted(array_form, len_array, &ind_first_error, &ind_second_error);
    while (!is_huffman_tree) {
        swap_nodes(array_form, ind_first_error, ind_second_error);
        recalculate_tree(array_form[ind_first_error], array_form[ind_second_error]);
        is_huffman_tree = is_sorted(array_form, len_array, &ind_first_error, &ind_second_error);
    }
}


//увеличение веса листа, перерасчет родителей
void update_tree(TreeNode *node, TreeNode *array_form[], int len_array) {
    while (node != NULL) {
        node->amount++;
        node = node->parent;
    }
    repair_tree(array_form, len_array);
    if (array_form[0]->amount == MAX_NODE_SIZE) {
        scale_tree(array_form[0]);
        repair_tree(array_form, len_array);
    }
}


void insert_node(TreeNode *node, TreeNode **nyt, TreeNode *array_form[], int *len_array) {
    TreeNode *old_nyt = *nyt;
    TreeNode *new_nyt = node_init(NYT, 0, NULL, NULL, old_nyt);
    new_nyt->parent = old_nyt;
    old_nyt->left = new_nyt;
    old_nyt->right = node;
    old_nyt->letter = EMPTY;
    old_nyt->amount = node->amount;
    node->parent = old_nyt;
    array_form[*len_array] = node;
    array_form[*len_array + 1] = new_nyt;
    *len_array += 2;
    *nyt = new_nyt;
}

void clear_tree(TreeNode *node) {
    if (node->left != NULL) {
        clear_tree(node->left);
    }
    if (node->right != NULL) {
        clear_tree(node->right);
    }
    free(node);
}


void compress(FILE *dest, FILE *source) {
    TreeNode *symbols[MAX_LEN] = {NULL};
    TreeNode *array_form[MAX_AMOUNT_NODES] = {NULL};
    TreeNode *nyt = node_init(NYT, 0, NULL, NULL, NULL);
    array_form[0] = nyt;
    int len_array = 1;
    TreeNode *end = node_init(END, 0, NULL, NULL, NULL);
    insert_node(end, &nyt, array_form, &len_array);
    Byte output_byte = {0};
    int symbol;
    Buffer *input_buff = buffer_init(source, 0);
    Buffer *output_buff = buffer_init(dest, BUFF_SIZE);
    while ((symbol = get_char(input_buff)) != EOF) {
        if (symbols[symbol] != NULL) {
            TreeNode *node = symbols[symbol];
            char code[MAX_LEN + 1] = "";
            get_code(node, code);
            update_byte(&output_byte, code, output_buff);
            update_tree(node, array_form, len_array);
        } else {
            char code[MAX_LEN + 1] = "";
            get_code(nyt, code);
            update_byte(&output_byte, code, output_buff);
            char code_symbol[LEN_BYTE + 1] = "";
            int_to_bin(code_symbol, symbol);
            update_byte(&output_byte, code_symbol, output_buff);
            TreeNode *new_node = node_init(symbol, 0, NULL, NULL, array_form[len_array - 1]);
            symbols[symbol] = new_node;
            insert_node(new_node, &nyt, array_form, &len_array);
            update_tree(new_node, array_form, len_array);
        }
    }
    char code[MAX_LEN + 1] = "";
    get_code(end, code);
    update_byte(&output_byte, code, output_buff);
    if (output_byte.count_filled != 0) {
        while (output_byte.count_filled % LEN_BYTE != 0) {
            output_byte.bits[output_byte.count_filled] = '0';
            output_byte.count_filled++;
        }
        put_char(output_buff, bin_to_int(output_byte.bits));
    }
    if (output_buff->size != 0) {
        print_buffer(output_buff);
    }
    clear_buffer(input_buff);
    clear_buffer(output_buff);
    clear_tree(array_form[0]);
}


void decompress(FILE *dest, FILE *source) {
    TreeNode *array_form[MAX_AMOUNT_NODES] = {NULL};
    TreeNode *nyt = node_init(NYT, 0, NULL, NULL, NULL);
    array_form[0] = nyt;
    int len_array = 1;
    TreeNode *end = node_init(END, 0, NULL, NULL, NULL);
    insert_node(end, &nyt, array_form, &len_array);
    bool is_ascii_code = false;
    Byte ascii_code = {0};
    TreeNode *curr_node = array_form[0];
    bool is_end = false;
    Buffer *input_buff = buffer_init(source, 0);
    Buffer *output_buff = buffer_init(dest, BUFF_SIZE);
    while (!is_end) {
        if (output_buff->size == output_buff->capacity) {
            print_buffer(output_buff);
        }
        char code[LEN_BYTE + 1] = "";
        int symbol = get_char(input_buff);
        int_to_bin(code, symbol);
        for (int i = 0; i < LEN_BYTE; i++) {
            if (!is_ascii_code) {
                if (code[i] == '0') {
                    curr_node = curr_node->left;
                } else {
                    curr_node = curr_node->right;
                }
            }
            if (ascii_code.count_filled == LEN_BYTE) {
                int letter = bin_to_int(ascii_code.bits);
                put_char(output_buff, letter);
                TreeNode *new_node = node_init(letter, 0, NULL, NULL, NULL);
                insert_node(new_node, &nyt, array_form, &len_array);
                update_tree(new_node, array_form, len_array);
                is_ascii_code = false;
                ascii_code.count_filled = 0;
                curr_node = array_form[0];
                i--;
            }
            if (is_ascii_code) {
                ascii_code.bits[ascii_code.count_filled] = code[i];
                ascii_code.count_filled++;
                continue;
            }
            if (curr_node->left == NULL && curr_node->right == NULL) {
                if (curr_node->letter != NYT) {
                    if (curr_node->letter == END) {
                        is_end = true;
                        break;
                    } else {
                        update_tree(curr_node, array_form, len_array);
                        put_char(output_buff, curr_node->letter);
                        curr_node = array_form[0];
                    }
                } else {
                    is_ascii_code = true;
                }
            }
        }
    }
    if (output_buff->size != 0) {
        print_buffer(output_buff);
    }
    clear_tree(array_form[0]);
    clear_buffer(output_buff);
    clear_buffer(input_buff);
}

void help(void) {
    printf("\nthe first parameter is type (it is either c - compress or d - decompress)\n"
           "the second parameter is a name of source file\nthe third parameter is a name of the result file\n");
    exit(EXIT_SUCCESS);
}

void syntax_error(void) {
    printf("\nsyntax error, program need 3 parameters, use -help to learn more\n");
    exit(EXIT_SUCCESS);
}

void check_file_name(char *name) {
    if (strstr(name, ".txt") == NULL) {
        printf("\ninvalid file name\n");
        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char *argv[]) {
    if (argc <= 1 || argc >= 5) {
        syntax_error();
    }
    if (argc == 2) {
        if (strcmp(argv[1], "-help") == 0) {
            help();
        } else {
            syntax_error();
        }
    }
    if (argc <= 3) {
        syntax_error();
    } else {
        if (strcmp(argv[1], "-c") != 0 && strcmp(argv[1], "-d") != 0) {
            printf("\nfirst argument must be -c or -d, use -help to read learn more\n");
            return 0;
        }
        check_file_name(argv[2]);
        check_file_name(argv[3]);
        FILE *source = fopen(argv[2], "rb");
        if (source == NULL) {
            printf("\nNo such file: %s\n", argv[2]);
            return 0;
        }
        FILE *destination = fopen(argv[3], "wb");
        signed char mode = argv[1][1];
        clock_t start_time = clock();
        switch (mode) {
            case zip: {
                printf("\nRunning...\n");
                compress(destination, source);
                clock_t total = clock() - start_time;
                printf("\nDone in %lf second(s)\n", (double) total / CLOCKS_PER_SEC);
                break;
            }
            case unzip: {
                printf("\nRunning...\n");
                decompress(destination, source);
                clock_t total = clock() - start_time;
                printf("\nDone in %lf second(s)\n", (double) total / CLOCKS_PER_SEC);
                break;
            }
            default: {
                printf("\nfirst argument must be -c or -d, use -help to read learn more\n");
                break;
            }
        }
        fclose(source);
        fclose(destination);
    }
}
