#ifndef TEXT_EDITOR_H
#define TEXT_EDITOR_H

#include <stdio.h>

#define ORIGINAL_BUFFER 0
#define ADD_BUFFER 1
#define ADD_CAPACITY 4096

// TextEditor structure - TODO: define your own fields here
typedef struct Piece {
  int buffer_type;
  int start;
  int length;
  struct Piece *next, *prev;
} Piece;

typedef struct TTextEditor {
  // TODO: define any fields you need for your implementation
  char *original_buffer;
  char *add_buffer;
  int original_size;
  int add_size, add_capacity;
  Piece *head, *tail;
  int global_cursor;
  int total_length;
} TextEditor;

// ============================================================================
// API Functions - You MUST implement all of these
// ============================================================================

// Create a new text editor
// filename: if not NULL, load content from this file
// text: if not NULL (and filename is NULL), initialize with this text
// global_cursor: initial cursor position
TextEditor *create_table(const char *filename, const char *text, int global_cursor);

// Move cursor by 'advance' positions (can be negative for backwards)
void advance_cursor(TextEditor *editor, int advance);

// Get current cursor position
int show_global_cursor(TextEditor *editor);

// Get total text length
int show_total_len(TextEditor *editor);

// Insert text at current cursor position
void add_text(TextEditor *editor, char *text);

// Delete 'length' characters before cursor
void delete_text(TextEditor *editor, int length);

// Extract full text as allocated string (caller must free)
char *extract_current_text(TextEditor *editor);

// Save text to file or return via text pointer
void save_current_text(TextEditor *editor, const char *filename, char **text, int *global_cursor);

// ============================================================================
// Helper functions (optional - you can define your own)
// ============================================================================
char* read_file(const char *filename, int *length);
Piece* create_piece(int buffer_type, int start, int len);
void insert_piece(TextEditor* editor, Piece* piece, Piece* new_piece);
void remove_piece(TextEditor* editor, Piece* piece);
void expand_buffer(TextEditor* editor, int space_needed);
void map_index_to_piece(TextEditor* editor, int index, Piece** target_piece,
                        int* offset_within_piece, Piece** before_piece);
void write_pieces(TextEditor* editor, FILE* file);


#endif // TEXT_EDITOR_H
