#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(errno);				        \
		}							\
	} while (0)

#define ALPHABET_SIZE 26
#define ALPHABET "abcdefghijklmnopqrstuvwxyz"

typedef struct trie_node_t trie_node_t;
struct trie_node_t {
	/* x if current node marks the end of a word
	that apears x times, 0 otherwise */
	int end_of_word;

	trie_node_t **children;

	//counts how many words contain this trie
	int n_words;
};

typedef struct trie_t trie_t;
struct trie_t {
	trie_node_t *root;

	/* Trie-Specific, alphabet properties */
	int alphabet_size;
	char *alphabet;

	/* Number of nodes, useful to test correctness */
	int n_nodes;
};

// functions used to create and initialize a node and trie
trie_node_t *create_node(void);
trie_t *create_trie(void);

// function used to insert a new word in the trie
void insert_node(trie_t *trie, char *word);

// function used to free the memory for one node
void free_node(trie_node_t *node);

// function that frees the hole trie
void free_trie(trie_t *trie);

// function that erifiers if a node has any children
int has_children(trie_node_t *node);

// function used to remove a word from the trie
void remove_node(trie_t *trie, char *word);

// functions used to verify and print the firs word from the trie
// that has no more than k different characters from the given word
void dfs_autocorrect(trie_node_t *node, char *current,
					 char *word, int k, int diff, int *ok);
void autocorrect(trie_t *trie, char *word, int k);

// function used to determine and display words that have the default
// prefix and meet various other requirements depending on the coomand
void dfs_lexico(trie_node_t *node, char *word,
				char *current, int poz, int *found);
void dfs_shortest(trie_node_t *node, const char *word, char
					 *current, int poz, int *shortest_len, char *shortest_word);
void dfs_max_frequency(trie_node_t *node, const char *word, char
						*current, int poz, int *max_freq, char *max_freq_w);
void autocomplete(trie_t *trie, char *word, int x);

int main(void)
{
	trie_t *trie = create_trie();
	char op[30];
	// read command
	scanf("%s", op);

	// while the exit command is not inserd, read commands
	while (strcmp(op, "EXIT") != 0) {
		char word[50];
		// read the word next to the command
		scanf("%s", word);

		if (strcmp(op, "LOAD") == 0) {
			// open file to read new words
			FILE *file = fopen(word, "r");
			DIE(!file, "Failed opening file\n");

			// insert the words into the trie
			char w[100];
			while (fscanf(file, "%s", w) != EOF)
				insert_node(trie, w);

			fclose(file);
		}

		// manage the special commands
		if (strcmp(op, "INSERT") == 0)
			insert_node(trie, word);

		if (strcmp(op, "REMOVE") == 0)
			remove_node(trie, word);

		if (strcmp(op, "AUTOCORRECT") == 0) {
			int k;
			scanf("%d", &k);
			autocorrect(trie, word, k);
		}

		if (strcmp(op, "AUTOCOMPLETE") == 0) {
			int x;
			scanf("%d", &x);
			autocomplete(trie, word, x);
		}

		// go to the next command
		scanf("%s", op);
	}

	free_trie(trie);
	return 0;
}

trie_node_t *create_node(void)
{
	// alloc an empty node
	trie_node_t *node = malloc(sizeof(trie_node_t));
	DIE(!node, "Malloc failed\n");

	// initialize the node's fields
	node->end_of_word = 0;
	node->children = malloc(ALPHABET_SIZE * sizeof(trie_node_t *));

	DIE(!node->children, "Malloc failed\n");
	node->n_words = 0;

	for (int i = 0; i < ALPHABET_SIZE; i++)
		node->children[i] = NULL;

	return node;
}

trie_t *create_trie(void)
{
	// create an empty trie
	trie_t *trie = malloc(sizeof(trie_t));
	DIE(!trie, "Malloc failed\n");

	// create the root node
	trie->root = create_node();

	trie->alphabet_size = ALPHABET_SIZE;
	trie->alphabet = malloc(trie->alphabet_size * sizeof(char));

	DIE(!trie->alphabet, "Malloc failed\n");
	for (int i = 0; i < trie->alphabet_size; i++)
		trie->alphabet[i] = ALPHABET[i];

	// the root is created
	trie->n_nodes = 1;

	return trie;
}

void insert_node(trie_t *trie, char *word)
{
	// start from the root node
	trie_node_t *current = trie->root;

	for (int i = 0; word[i] != '\0'; i++) {
		// iterate through each character in the word
		// and insert them into the trie
		int index = word[i] - 'a';

		// if the current letter does not exists add a new node
		if (!current->children[index]) {
			current->children[index] = create_node();
			trie->n_nodes++;
		}

		current->n_words++;
		current = current->children[index];
	}

	current->end_of_word++;
	current->n_words++;
}

void free_node(trie_node_t *node)
{
	if (!node)
		return;

	for (int i = 0; i < ALPHABET_SIZE; i++)
		free_node(node->children[i]);

	free(node->children);
	free(node);
}

void free_trie(trie_t *trie)
{
	if (!trie)
		return;

	free_node(trie->root);
	free(trie->alphabet);
	free(trie);
}

int has_children(trie_node_t *node)
{
	for (int i = 0; i < ALPHABET_SIZE; i++)
		if (node->children[i])
			return 1;

	return 0;
}

void remove_node(trie_t *trie, char *word)
{
	trie_node_t *current = trie->root;
	trie_node_t *parent = NULL;
	int index;

	for (int i = 0; word[i] != '\0'; i++) {
		// iterate through each character in the word
		index = word[i] - 'a';

		if (!current)
			return;

		current->n_words--;
		// update the parent node and move to the next one
		parent = current;
		current = current->children[index];
	}

	if (!current || current->end_of_word == 0)
		return;

	// set the end_of_word flag of the current node to 0,
	// since it is no longer the end of a word
	current->end_of_word = 0;
	current->n_words--;

	if (has_children(current))
		return;

	// iterate while the parent exists, has no children,
	// and is not the end of a word
	while (parent && !has_children(parent) &&
		   parent->end_of_word == 0) {
		current = parent;
		parent = NULL;

		for (int i = 0; word[i] != '\0'; i++) {
			index = word[i] - 'a';

			// if the current node is the one to be deleted
			if (current->children[index] == current) {
				current->children[index] = NULL;
				break;
			}

			// go to the next node
			parent = current;
			current = current->children[index];
		}

		free(current);
	}
}

void dfs_autocorrect(trie_node_t *node, char *current,
					 char *word, int k, int diff, int *ok)
{
	if (!node)
		return;

	if (node->end_of_word > 0 && k - diff >= 0 &&
		strlen(word) == strlen(current)) {
		printf("%s\n", current);
		*ok = 0;
	}

	for (int i = 0; i < 26; i++) {
		if (node->children[i]) {
			// create a new word by appending the current
			// character to the existing word
			char c = 'a' + i;

			int length = strlen(current);
			char *new_word = (char *)malloc(sizeof(char) * (length + 2));
			strcpy(new_word, current);
			new_word[length] = c;
			new_word[length + 1] = '\0';

			// calculate the number of differences between
			// the new word and the given word
			int diff = 0;
			for (int j = 0; j < length + 1; j++)
				if (new_word[j] != word[j])
					diff++;

			// recursively call the function with the new
			// word and updated parameters
			dfs_autocorrect(node->children[i], new_word, word, k, diff, ok);

			free(new_word);
		}
	}
}

void autocorrect(trie_t *trie, char *word, int k)
{
	// alloc memory for the new word
	int length = strlen(word), ok = 1;
	char *current = (char *)malloc(sizeof(char) * (length + 1));
	current[0] = '\0';

	// trie to find the word that has only k different
	// characters form the given word
	dfs_autocorrect(trie->root, current, word, k, 0, &ok);

	// if ok hasn't changed its value, print the correct message
	if (ok == 1)
		printf("No words found\n");

	free(current);
}

void dfs_lexico(trie_node_t *node, char *word,
				char *current, int poz, int *found)
{
	if (!node)
		return;

	// if the current node marks the end of a word and it's
	// lenght is at least equal to the length of the given word,
	// print the current word.
	if (node->end_of_word > 0 && poz >= strlen(word)) {
		printf("%s\n", current);
		// if we have discoverd a word, there is no need to print another one
		*found = 0;
		return;
	}

	// update the current word by adding characters to it
	for (int i = 0; i < 26; i++) {
		// if the position of the current character is in the
		// range [0, lenght(target word)], then we switch to
		// the next character of the target word if it currently
		// exists in the string
		if (poz < strlen(word)) {
			if (node->children[i] && 'a' + i == word[poz]) {
				char c = 'a' + i;
				int length = strlen(current);
				char *new_word = (char *)malloc(sizeof(char) * (length + 2));
				strcpy(new_word, current);
				new_word[length] = c;
				new_word[length + 1] = '\0';

				dfs_lexico(node->children[i], word, new_word, ++poz, found);
				free(new_word);

				// if found = 0, it means that we already found the correct word
				if (*found == 0)
					return;
			}
		} else {
			// add any other existing letters from the trie
			if (node->children[i]) {
				char c = 'a' + i;
				int length = strlen(current);
				char *new_word = (char *)malloc(sizeof(char) * (length + 2));
				strcpy(new_word, current);
				new_word[length] = c;
				new_word[length + 1] = '\0';

				dfs_lexico(node->children[i], word, new_word, ++poz, found);
				free(new_word);

				// if found = 0, it means that we already found the correct word
				if (*found == 0)
					return;
			}
		}
	}
}

void dfs_shortest(trie_node_t *node, const char *word,
				  char *current, int poz, int *shortest_len, char *shortest_w)
{
	if (!node)
		return;

	// if the current node marks the end of a word and it's
	// lenght is at least equal to the length of the given word,
	// print the current word.
	if (node->end_of_word > 0 && poz >= strlen(word)) {
		// verify if the current word is the shortest
		if (strlen(current) < *shortest_len) {
			strcpy(shortest_w, current);
			*shortest_len = strlen(current);
		}
		return;
	}

	// update the current word by adding characters to it
	for (int i = 0; i < 26; i++) {
		if (poz < strlen(word)) {
			// if the position of the current character is in the
			// range [0, lenght(target word)], then we switch to
			// the next character of the target word if it currently
			// exists in the string
			if (node->children[i] && 'a' + i == word[poz]) {
				char c = 'a' + i;
				int length = strlen(current);
				char *new_word = (char *)malloc(sizeof(char) * (length + 2));
				strcpy(new_word, current);
				new_word[length] = c;
				new_word[length + 1] = '\0';

				dfs_shortest(node->children[i], word, new_word,
							 poz + 1, shortest_len, shortest_w);
				free(new_word);
			}
		} else {
			// add any other existing letters from the trie
			if (node->children[i]) {
				char c = 'a' + i;
				int length = strlen(current);
				char *new_word = (char *)malloc(sizeof(char) * (length + 2));
				strcpy(new_word, current);
				new_word[length] = c;
				new_word[length + 1] = '\0';

				dfs_shortest(node->children[i], word, new_word,
							 poz + 1, shortest_len, shortest_w);

				free(new_word);
			}
		}
	}
}

void dfs_max_frequency(trie_node_t *node, const char *word,
					   char *current, int poz, int *max_freq, char *max_freq_w)
{
	if (!node)
		return;

	// if the current node marks the end of a word and it's
	// lenght is at least equal to the length of the given word,
	// print the current word.
	if (node->end_of_word > 0 && poz >= strlen(word))
		// verify if the current word's frequency is gratern than the max
		// the frequency of each word is stored in end_of_word
		if (node->end_of_word > *max_freq) {
			strcpy(max_freq_w, current);
			*max_freq = node->end_of_word;
		}

	// update the current word by adding characters to it
	for (int i = 0; i < 26; i++) {
		if (poz < strlen(word)) {
			// if the position of the current character is in the
			// range [0, lenght(target word)], then we switch to
			// the next character of the target word if it currently
			// exists in the string
			if (node->children[i] && 'a' + i == word[poz]) {
				char c = 'a' + i;
				int length = strlen(current);
				char *new_word = (char *)malloc(sizeof(char) * (length + 2));
				strcpy(new_word, current);
				new_word[length] = c;
				new_word[length + 1] = '\0';

				dfs_max_frequency(node->children[i], word,
								  new_word, poz + 1, max_freq, max_freq_w);
				free(new_word);
			}
		} else {
			// add any other existing letters from the trie
			if (node->children[i]) {
				char c = 'a' + i;
				int length = strlen(current);
				char *new_word = (char *)malloc(sizeof(char) * (length + 2));
				strcpy(new_word, current);
				new_word[length] = c;
				new_word[length + 1] = '\0';

				dfs_max_frequency(node->children[i], word,
								  new_word, poz + 1, max_freq, max_freq_w);
				free(new_word);
			}
		}
	}
}

void autocomplete(trie_t *trie, char *word, int x)
{
	// initialize the words to be searched
	char current[50], shortest_len_w[50], max_frec_w[50];
	int found = 1, shortest_len = 1000, max = 0;
	current[0] = '\0';
	shortest_len_w[0] = '\0';
	max_frec_w[0] = '\0';

	// depending on the value of x, call the functions of interest
	if (x == 0) {
		dfs_lexico(trie->root, word, current, 0, &found);
		if (found == 1)
			printf("No words found\n");

		dfs_shortest(trie->root, word, current, 0,
					 &shortest_len, shortest_len_w);
		if (strcmp(shortest_len_w, "\0") == 0)
			printf("No words found\n");
		else
			printf("%s\n", shortest_len_w);

		dfs_max_frequency(trie->root, word, current, 0, &max, max_frec_w);
		if (strcmp(max_frec_w, "\0") == 0)
			printf("No words found\n");
		else
			printf("%s\n", max_frec_w);
	}

	if (x == 1) {
		dfs_lexico(trie->root, word, current, 0, &found);
		if (found == 1)
			printf("No words found\n");
	}

	if (x == 2) {
		dfs_shortest(trie->root, word, current, 0,
					 &shortest_len, shortest_len_w);
		if (strcmp(shortest_len_w, "\0") == 0)
			printf("No words found\n");
		else
			printf("%s\n", shortest_len_w);
	}
	if (x == 3) {
		dfs_max_frequency(trie->root, word, current, 0, &max, max_frec_w);
		if (strcmp(max_frec_w, "\0") == 0)
			printf("No words found\n");
		else
			printf("%s\n", max_frec_w);
	}
}
