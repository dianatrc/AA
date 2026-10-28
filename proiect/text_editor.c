#include "text_editor.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

TextEditor *create_table(const char *filename, const char *text,
                         int global_cursor) {
  TextEditor* editor = (TextEditor*)malloc(sizeof(TextEditor));
  if (!editor) {
    printf("eroare la alocare");
    return NULL;
  }

  char *content = NULL;
  int content_length = 0;

  if (filename) {
    content = read_file(filename, &content_length);
    if (!content) {
      content = strdup("");
      content_length = 0;
    }
  } else if (text) {
    content = (char*)malloc(strlen(text) + 1);
    if (content) {
      content_length = strlen(text);
      memcpy(content, text, content_length);
      content[content_length] = '\0';
    } else {
      content = strdup("");
      content_length = 0;
    }
  } else {
    content = strdup("");
    content_length = 0;
  }

  editor->original_buffer = content;
  editor->original_size = content_length;

  editor->add_capacity = ADD_CAPACITY;
  editor->add_buffer = (char*)malloc(ADD_CAPACITY);
  if (!editor->add_buffer) {
    free(editor->original_buffer);
    free(editor);
    return NULL;
  }
  editor->add_size = 0;

  editor->head = NULL;
  editor->tail = NULL;

  editor->total_length = content_length;
  if (global_cursor < 0)
    editor->global_cursor = 0;
  else if (global_cursor > editor->total_length)
    editor->global_cursor = editor->total_length;
  else
    editor->global_cursor = global_cursor;

  if (content_length > 0) {
    Piece* piece = create_piece(ORIGINAL_BUFFER, 0, content_length);
    if (piece) {
      editor->head = piece;
      editor->tail = piece;
    }
  }
  return editor;
}

void advance_cursor(TextEditor *editor, int advance) {
  editor->global_cursor += advance;
  if (editor->global_cursor < 0)
    editor->global_cursor = 0;
  else if (editor->global_cursor > editor->total_length)
    editor->global_cursor = editor->total_length;
}

int show_global_cursor(TextEditor *editor) {
  return editor->global_cursor;
}

int show_total_len(TextEditor *editor) {
  return editor->total_length;
}

void add_text(TextEditor *editor, char *text) {
  if (!editor || !text)
    return;
  int space_needed = strlen(text);
  if (space_needed == 0)
    return;
  expand_buffer(editor, space_needed);

  int add_start = editor->add_size;
  memcpy(editor->add_buffer + add_start, text, space_needed);
  editor->add_size += space_needed;
  
  Piece* target_piece;
  int offset_within_piece;
  Piece* before_piece;
  map_index_to_piece(editor, editor->global_cursor, &target_piece, &offset_within_piece, &before_piece);

  Piece* new_piece = create_piece(ADD_BUFFER, add_start, space_needed);
  if (!new_piece)
    return;
  
  if (target_piece == NULL) {
    // add at the end
    insert_piece(editor, before_piece, new_piece);
  } else if (offset_within_piece == 0) {
    // add at the begginning of piece
    insert_piece(editor, before_piece, new_piece);
  } else if (offset_within_piece == target_piece->length) {
    // add at the end of piece
    insert_piece(editor, target_piece, new_piece);
  } else {
    // add in the middle of piece
    Piece* second_half = create_piece(target_piece->buffer_type, target_piece->start + offset_within_piece,
                                target_piece->length - offset_within_piece);
    if (!second_half) {
      free(new_piece);
      return;
    }
    target_piece->length = offset_within_piece;
    insert_piece(editor, target_piece, new_piece);
    insert_piece(editor, new_piece, second_half);
  }

  editor->global_cursor += space_needed;
  editor->total_length += space_needed;
}

void delete_text(TextEditor *editor, int length) {
  if (!editor || length <= 0 || editor->global_cursor == 0)
    return;

  if (length > editor->global_cursor)
    length = editor->global_cursor;
  
  int start_delete = editor->global_cursor - length;
  int end_delete = editor->global_cursor;

  Piece* current_piece = editor->head;
  int pos = 0;
  while (current_piece) {
    int piece_end = current_piece->length + pos;
    Piece* next = current_piece->next;

    // calculate intersection between piece and delete range
    int start_overlap, end_overlap;
    if (pos > start_delete)
      start_overlap = pos;
    else start_overlap = start_delete;
    if  (piece_end < end_delete)
      end_overlap = piece_end;
    else end_overlap = end_delete;

    if (start_overlap < end_overlap) {
      // delete within current piece
      int overlap_len = end_overlap - start_overlap;
      if (pos >= start_delete && piece_end <= end_delete)
        // delete whole piece
        remove_piece(editor, current_piece);
      else if (pos < start_delete && piece_end > end_delete) {
        // deletion within piece - split piece
        int left_len = start_delete - pos;
        int right_len = piece_end - end_delete;

        current_piece->length = left_len;
        Piece* right_piece = create_piece(current_piece->buffer_type, current_piece->start +
                                          (end_delete - pos), right_len);
        if (!right_piece)
          return;
        insert_piece(editor, current_piece, right_piece);
      } else if (pos < start_delete && piece_end <= end_delete) {
        // truncate right side
        current_piece->length = start_delete - pos;
      } else if (pos >= start_delete && piece_end > end_delete) {
        // adjust start - truncate left side
        current_piece->start += overlap_len;
        current_piece->length -= overlap_len;
      }
    }
    pos = piece_end;
    current_piece = next;
    if (pos >= end_delete)
      break;
  }

  editor->total_length -= length;
  editor->global_cursor  -=length;
}

char *extract_current_text(TextEditor *editor) {
  if (!editor)
    return NULL;
  if (editor->total_length == 0)
    return strdup("");

  char* result = (char*)malloc(editor->total_length + 1);
  if (!result)
    return NULL;
  
  int pos = 0;
  for (Piece* current_piece = editor->head; current_piece; current_piece = current_piece->next) {
    if (current_piece->buffer_type == ORIGINAL_BUFFER)
      memcpy(result + pos, editor->original_buffer + current_piece->start, current_piece->length);
    else
      memcpy(result + pos, editor->add_buffer + current_piece->start, current_piece->length);
    pos += current_piece->length;
  }
  result[editor->total_length] = '\0';
  return result;
}

void save_current_text(TextEditor *editor, const char *filename, 
                       char **text, int *global_cursor) {
  if (!editor)
    return;
  
  if (global_cursor)
    *global_cursor = editor->global_cursor;
  if (text)
    *text = extract_current_text(editor);
  
  if (!filename)
    return;

  FILE* file = fopen(filename, "w");
  if(!file)
    return;
  write_pieces(editor, file);
  fclose(file);
}

char* read_file(const char *filename, int *length) {
  FILE* file = fopen(filename, "rb");
  if (!file) {
    *length = 0;
    return NULL;
  }

  fseek(file, 0 , SEEK_END); // end of file
  int file_size = ftell(file);
  fseek(file, 0, SEEK_SET); // start of file

  char* content = (char*)malloc(file_size + 1);
  if (!content) {
    fclose(file);
    *length = 0;
    return NULL;
  }

  int bytes_read = fread(content, 1, file_size, file);
  content[bytes_read] = '\0';
  *length = bytes_read;
  fclose(file);

  return content;
}

Piece* create_piece(int buffer_type, int start, int len) {
  Piece* piece = (Piece*)malloc(sizeof(Piece));
  if (!piece) {
    printf("eroare la alocare\n");
    return NULL;
  }
  piece->buffer_type = buffer_type;
  piece->length = len;
  piece->start = start;
  piece->next = NULL;
  piece->prev = NULL;
  return piece;
}

void insert_piece(TextEditor* editor, Piece* piece, Piece* new_piece) {
  if (!piece) {
    // insert first piece
    new_piece->next = editor->head;
    new_piece->prev = NULL;

    if (editor->head)
      editor->head->prev = new_piece;
    editor->head = new_piece;
    if (!editor->tail)
      editor->tail = new_piece;
  } else {
    // insert after piece
    new_piece->next = piece->next;
    new_piece->prev = piece;
    if (piece->next)
      piece->next->prev = new_piece;
    else
      editor->tail = new_piece;
    piece->next = new_piece;
  }
}

void remove_piece(TextEditor* editor, Piece* piece) {
  if (piece->prev)
    piece->prev->next = piece->next;
  else editor->head = piece->next;

  if (piece->next)
    piece->next->prev = piece->prev;
  else editor->tail = piece->prev;

  free(piece);
}

void expand_buffer(TextEditor* editor, int space_needed) {
  if (editor->add_size + space_needed <= editor->add_capacity)
    return;
  int new_capacity = editor->add_capacity;
  while (new_capacity < editor->add_size + space_needed)
    new_capacity *= 2;
  char* new_buffer = (char*)realloc(editor->add_buffer, new_capacity);
  if (!new_buffer)
    return;
  editor->add_buffer = new_buffer;
  editor->add_capacity = new_capacity;
}

void map_index_to_piece(TextEditor* editor, int index, Piece** target_piece,
                        int* offset_within_piece, Piece** before_piece) {
  int current_pos = 0;
  Piece* current_piece = editor->head;
  Piece* prev = NULL;

  while (current_piece) {
    if (index <= current_pos + current_piece->length) {
      *target_piece = current_piece;
      *before_piece = prev;
      *offset_within_piece = index - current_pos;
      return;
    }
    current_pos += current_piece->length;
    prev = current_piece;
    current_piece = current_piece->next;
  }

  *target_piece = NULL;
  *before_piece = editor->tail;
  *offset_within_piece = 0;
}

void write_pieces(TextEditor* editor, FILE* file) {
  for (Piece* current = editor->head; current; current = current->next) {
    if (current->buffer_type == ORIGINAL_BUFFER)
      fwrite(editor->original_buffer + current->start, 1, current->length, file);
    else
      fwrite(editor->add_buffer + current->start, 1, current->length, file);
  }
}

