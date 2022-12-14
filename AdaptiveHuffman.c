#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>


#define EMPTY (-2)
#define NYT (-3)
#define END (-4)
#define MAX_LEN 256
#define MAX_AMOUNT_NODES ((MAX_LEN + 2) * 2 - 1)
#define LEN_BYTE 8
#define MAX_NODE_SIZE SHRT_MAX
#define BUFF_SIZE 10000


typedef struct byte {
    int count_filled;
    char bits[LEN_BYTE + 1];
} byte;

typedef struct tree_node {
    int letter;
    int amount;
    struct tree_node *right;
    struct tree_node *left;
    struct tree_node *parent;
} tree_node;


typedef struct queue {
    tree_node *array[MAX_AMOUNT_NODES];
    int first;
    int last;
} queue;


enum mode {
    zip = (int) 'c',
    unzip = (int) 'd',
};


tree_node memory[MAX_AMOUNT_NODES];
short int filled_memory = 0;
unsigned char buffer[BUFF_SIZE + 1] = "";
short int len_buffer = 0;
short int amount_read = 0;


tree_node *extract(queue *my_queue) {
    if (my_queue->last - my_queue->first > 0) {
        tree_node *node = my_queue->array[my_queue->first % MAX_AMOUNT_NODES];
        my_queue->first++;
        return node;
    } else {
        return NULL;
    }
}


void add(queue *my_queue, tree_node *node) {
    my_queue->array[my_queue->last % MAX_AMOUNT_NODES] = node;
    my_queue->last++;
}


bool is_empty(queue *my_queue) {
    return my_queue->last - my_queue->first == 0;
}


int min(int first, int second) {
    return (first < second) ? first : second;
}


int get_char(FILE *stream) {
    if (amount_read == len_buffer) {
        len_buffer = (short int) fread(buffer, sizeof(unsigned char), BUFF_SIZE, stream);
        amount_read = 0;
    }
    if (amount_read < len_buffer) {
        amount_read++;
        return buffer[amount_read - 1];
    } else {
        return -1;
    }
}


void swap(void *el1, void *el2, int size) {
    for (int i = 0; i < size; i++) {
        char buf = *((char *) el1 + i);
        *((char *) el1 + i) = *((char *) el2 + i);
        *((char *) el2 + i) = buf;
    }
}


void rebuild_array(tree_node **array_form, int ind_start) {
    int curr_ind = 0;
    queue my_queue = {{0}, 0, 0};
    add(&my_queue, array_form[0]);
    while (!is_empty(&my_queue)) {
        tree_node *node = extract(&my_queue);
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


void swap_nodes(tree_node **array_form, int ind_first_error, int ind_second_error) {
    tree_node *node1 = array_form[ind_first_error];
    tree_node *node2 = array_form[ind_second_error];
    if (node1->parent == node2->parent) {
        tree_node *left = (node1->parent->left == node1) ? node1 : node2;
        tree_node *right = (node1->parent->right == node1) ? node1 : node2;
        tree_node *parent = node1->parent;
        parent->left = right;
        parent->right = left;
    } else {
        tree_node *parent1 = node1->parent;
        tree_node *parent2 = node2->parent;
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
    if (node1->left != NULL || node2->left != NULL || node2->right != NULL || node1->right != NULL) {
        int ind_start = min(ind_first_error, ind_second_error);
        rebuild_array(array_form, ind_start);
    } else {
        swap(&array_form[ind_first_error], &array_form[ind_second_error], sizeof(tree_node **));
    }
}


tree_node *node_init(int letter, int amount, tree_node *left, tree_node *right, tree_node *parent) {
    tree_node *new_node = &memory[filled_memory];
    filled_memory++;
    *new_node = (tree_node) {letter, amount, left, right, parent};
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


void update_byte(byte *output_byte, char code[], FILE *stream) {
    int len_code = (short int) strlen(code);
    for (int i = 0; i < len_code; i++) {
        output_byte->bits[output_byte->count_filled] = code[i];
        output_byte->count_filled++;
        if (output_byte->count_filled == 8) {
            fprintf(stream, "%c", bin_to_int(output_byte->bits));
            output_byte->count_filled = 0;
        }
    }
}


int scale_tree(tree_node *node) {
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


void update_parents(tree_node *node) {
    while (node->parent != NULL) {
        tree_node *parent = node->parent;
        if (parent->right == node) {
            parent->amount = parent->left->amount + node->amount;
        } else {
            parent->amount = parent->right->amount + node->amount;
        }
        node = parent;
    }
}


void recalculate_tree(tree_node *first_node, tree_node *second_node) {
    update_parents(first_node);
    update_parents(second_node);
}


void get_code(tree_node *node, char *dest) {
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


bool is_sorted(tree_node *array_form[], int len_array, int *first_ind, int *second_ind) {
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


void repair_tree(tree_node *array_form[], int len_array) {
    int ind_first_error;
    int ind_second_error;
    bool is_huffman_tree = is_sorted(array_form, len_array, &ind_first_error, &ind_second_error);
    while (!is_huffman_tree) {
        swap_nodes(array_form, ind_first_error, ind_second_error);
        recalculate_tree(array_form[ind_first_error], array_form[ind_second_error]);
        is_huffman_tree = is_sorted(array_form, len_array, &ind_first_error, &ind_second_error);
    }
}


void update_tree(tree_node *node, tree_node *array_form[], int len_array) {
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


void insert_node(tree_node *node, tree_node **nyt, tree_node *array_form[], int *len_array) {
    tree_node *old_nyt = *nyt;
    tree_node *new_nyt = node_init(NYT, 0, NULL, NULL, old_nyt);
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


void compress(FILE *dest, FILE *source) {
    tree_node *symbols[MAX_LEN] = {NULL};
    tree_node *array_form[MAX_AMOUNT_NODES] = {NULL};
    tree_node *nyt = node_init(NYT, 0, NULL, NULL, NULL);
    array_form[0] = nyt;
    int len_array = 1;
    tree_node *end = node_init(END, 0, NULL, NULL, NULL);
    insert_node(end, &nyt, array_form, &len_array);
    byte output_byte = {0};
    int symbol;
    while ((symbol = get_char(source)) != EOF) {
        if (symbols[symbol] != NULL) {
            tree_node *node = symbols[symbol];
            char code[MAX_LEN + 1] = "";
            get_code(node, code);
            update_byte(&output_byte, code, dest);
            update_tree(node, array_form, len_array);
        } else {
            char code[MAX_LEN + 1] = "";
            get_code(nyt, code);
            update_byte(&output_byte, code, dest);
            char code_symbol[LEN_BYTE + 1] = "";
            int_to_bin(code_symbol, symbol);
            update_byte(&output_byte, code_symbol, dest);
            tree_node *new_node = node_init(symbol, 0, NULL, NULL, array_form[len_array - 1]);
            symbols[symbol] = new_node;
            insert_node(new_node, &nyt, array_form, &len_array);
            update_tree(new_node, array_form, len_array);
        }
    }
    char code[MAX_LEN + 1] = "";
    get_code(end, code);
    update_byte(&output_byte, code, dest);
    if (output_byte.count_filled != 0) {
        while (output_byte.count_filled % LEN_BYTE != 0) {
            output_byte.bits[output_byte.count_filled] = '0';
            output_byte.count_filled++;
        }
        fprintf(dest, "%c", bin_to_int(output_byte.bits));
    }
}


void decompress(FILE *dest, FILE *source) {
    tree_node *array_form[MAX_AMOUNT_NODES] = {NULL};
    tree_node *nyt = node_init(NYT, 0, NULL, NULL, NULL);
    array_form[0] = nyt;
    int len_array = 1;
    tree_node *end = node_init(END, 0, NULL, NULL, NULL);
    insert_node(end, &nyt, array_form, &len_array);
    bool is_ascii_code = false;
    byte ascii_code = {0};
    tree_node *curr_node = array_form[0];
    bool is_end = false;
    while (!is_end) {
        char code[LEN_BYTE + 1] = "";
        int symbol = get_char(source);
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
                fprintf(dest, "%c", letter);
                tree_node *new_node = node_init(letter, 0, NULL, NULL, NULL);
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
                        fprintf(dest, "%c", curr_node->letter);
                        curr_node = array_form[0];
                    }
                } else {
                    is_ascii_code = true;
                }
            }
        }
    }
}


void get_output_name(char *output_name, char *input_name, int mode) {
    if (mode == zip) {
        strncpy(output_name, input_name, strchr(input_name, '.') - input_name);
        strcat(output_name, "_compressed.txt");
    } else {
        if (strstr(input_name, "_compressed") == NULL) {
            strncpy(output_name, input_name, strchr(input_name, '.') - input_name);
            strcat(output_name, "_uncompressed.txt");
        } else {
            strncpy(output_name, input_name, strstr(input_name, "_compressed") - input_name);
            strcat(output_name, ".txt");
        }
    }
}


void get_input_name(char *input_name) {
    fgets(input_name, MAX_LEN, stdin);
    input_name[strlen(input_name) - 1] = '\000';
}


int main(void) {
    char input_name[MAX_LEN] = "";
    char output_name[MAX_LEN] = "";
    printf("Enter file name: ");
    get_input_name(input_name);
    printf("\rEnter mode (compress(c) / decompress(d)): ");
    int mode = getchar();
    get_output_name(output_name, input_name, mode);
    FILE *source = fopen(input_name, "rb");
    FILE *destination = fopen(output_name, "wb");
    if (source == NULL && destination == NULL) {
        printf("Syntax error, check and try again");
        exit(0);
    }
    switch (mode) {
        case zip: {
            compress(destination, source);
            printf("\rfile was successfully compressed\n");
            break;
        }
        case unzip: {
            decompress(destination, source);
            printf("\rfile was successfully decompressed\n");
            break;
        }
        default: {
            printf("Syntax error");
            break;
        }
    }
    system("pause");
    fclose(source);
    fclose(destination);
}
