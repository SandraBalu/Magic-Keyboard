# Magic-Keyboard

We plan to implement a word correction/completion system for a user based on their history (words used in the past). For the implementation of this program we will use a trie, thus making the following operations both memory efficient and complex: INSERT, LOAD, REMOVE, AUTOCORRECT, AUTOCOMPLETE, EXIT.

## INSERT <word>
Retain the word or increment the occurrence frequency within the data structure.

## LOAD <filename>
Read an ascii file (it will contain lowercase letters of the English alphabet and whitespace), and insert all the words from it into the data structure. This file models the user's history. (It is guaranteed that the file exists and is formatted correctly.)

## REMOVE <word> 
We remove the word (and free the memory used by it) from the data structure (it can still be re-added in the future).

## AUTOCORRECT <word> <k> 
Require all words that differ from the given word, in up to k characters. We will only consider letter changes, not insertions or deletions. All words will be displayed in lexicographic order.


## AUTOFILL <prefix> <n> 
For this task, you have to display 3 words:

    n = 1 : The smallest lexicographically given prefix word
    n = 2 : Shortest word with the given prefix
    n = 3 : Most frequently used word with the given prefix (in case of a tie, the smallest lexicographically)
  
If  n = 0, we will display all 3 criteria.

## EXIT
The used memory is freed and the program ends



