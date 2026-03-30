#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ROWS 3
#define COLS 3

#define GEMCHOSENCOUNT 2

static int _window_width = 700;
static int _window_height = 700;
static int _target_fps = 30;
static Color _background_color = BLACK;

static int _board_beginning_position_x = 150;
static int _board_beginning_position_y = 90;

static int _rectangle_width = 75;
static int _rectangle_height = 75;
static int _rectangle_row_space = 5;
static int _rectangle_cols_space = 5;
static Color _rectangle_color = GREEN;

static int _circle_radius = 25;

static Color _gem_type_color[] = {RED, BLUE, LIME, GOLD};



struct Gem{
  int type;
  Color color;
  int order; 
  int pos_y;
  int pos_x;
  int index_x;
  int index_y;
};

struct Gem gems[ROWS][COLS];

struct Cell{
  int order;
  int pos_x;
  int pos_y;
  int index_x;
  int index_y;
  struct Gem gem;
};

struct Cell cell[ROWS][COLS];


struct Chosen_Gems{
  bool gem_assigned;
  struct Gem _chosen_gem;
};

struct Chosen_Gems _chosen_gems[GEMCHOSENCOUNT];

static void _initialize_window() {
  InitWindow(_window_height, _window_height, "Raylib testing");
  SetTargetFPS(_target_fps);
}

static void _draw_board() {

  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      DrawRectangle(cell[i][j].pos_x, cell[i][j].pos_y, _rectangle_width,_rectangle_height, _rectangle_color);
    }
  }
}

static void _draw_gems() {
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      DrawCircle(gems[i][j].pos_x, gems[i][j].pos_y, _circle_radius, gems[i][j].color);
    }
  }
}

static void _update_gem_position_based_on_cell(int i, int j)
{
  gems[i][j].pos_x = cell[i][j].pos_x + _rectangle_width / 2;
  gems[i][j].pos_y = cell[i][j].pos_y + _rectangle_height / 2;
  gems[i][j].order = cell[i][j].order;
  gems[i][j].index_x = cell[i][j].index_x;
  gems[i][j].index_y = cell[i][j].index_y;
}

static void _initialize_game() {
  // Creating randomized gams are type between 1 and 4;
  // Initializing board
  int _current_board_x_position = _board_beginning_position_x;
  int _current_board_y_position = _board_beginning_position_y;

  srand(time(NULL));

  int step = 0;
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {

      //BOARD section
      cell[i][j].index_x = i;
      cell[i][j].index_y = j;
      cell[i][j].order = step;

      cell[i][j].pos_y = _current_board_y_position;
      cell[i][j].pos_x = _current_board_x_position;
      _current_board_x_position += _rectangle_cols_space + _rectangle_width;


      //GEMS section
      gems[i][j].type = rand() % 4 + 1;
      gems[i][j].color = _gem_type_color[gems[i][j].type - 1];
      _update_gem_position_based_on_cell(i, j); //Updating gem's properties based on it's cell
      
      step++;
    }

    _current_board_x_position = _board_beginning_position_x;
    _current_board_y_position += _rectangle_row_space + _rectangle_height;
  }

  //Clean junk values
  for(int i = 0; i < GEMCHOSENCOUNT; i++){
    _chosen_gems[i].gem_assigned = false;
  }
}



/*
// This is the old version of _gem_return_on_mouse_click
static void _gem_return_on_mouse_click_deprecated() {
  Vector2 mouse_position = GetMousePosition();
  printf("%f %f", GetMousePosition().x, GetMousePosition().y);
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      if (mouse_position.x > gems[i][j].cell_pos_x &&
          mouse_position.x < gems[i][j].cell_pos_x + _rectangle_width) {
        if (mouse_position.y > gems[i][j].cell_pos_y &&
            mouse_position.y < gems[i][j].cell_pos_y + _rectangle_height) {
          printf("You clicked: %d\n", gems[i][j].gem_order);
          return;
        }
      }
    }
  }
  printf("You clicked nowhere\n");
}*/

/**
 * @brief Translates screen-space mouse coordinates into 2D grid indices.
 * * @param None (Uses Raylib's GetMousePosition internally).
 * @return void (Currently logs the clicked cell index to the console).
 * @note Currently the gap between gems are clickable but, it's intentionally left
 */
static struct Gem _gem_return_on_mouse_click() {
  Vector2 mouse_position_normalized = 
  (Vector2){GetMousePosition().x - _board_beginning_position_x,
            GetMousePosition().y - _board_beginning_position_y};

  int index_x = (int)((mouse_position_normalized.y / (float)(_rectangle_height + _rectangle_row_space)));
  int index_y = (int)((mouse_position_normalized.x / (float)(_rectangle_width + _rectangle_cols_space)));

  if (index_x < ROWS && index_y < COLS
     && index_x >= 0 && index_y >= 0
     && mouse_position_normalized.x >= 0 && mouse_position_normalized.y >= 0) {
    printf("You clicked: %d\n", gems[index_x][index_y].order);
    return gems[index_x][index_y];
  }
}

/**
 * @brief Process information of last chosen 2 gems to keep in array
 * 
 */
static void _gem_choser(struct Gem gem){
  for(int i = 0; i < GEMCHOSENCOUNT; i++){
    if(_chosen_gems[i].gem_assigned == false){
      _chosen_gems[i]._chosen_gem = gem;
      _chosen_gems[i].gem_assigned = true;
      break;
    }
  }
}

static void _swap_gems(){
  for(int i = 0; i < ROWS; i++){
    for(int j = 0; j < COLS; j++){
      printf("%d", gems[i][j].type);
    }
    printf("\n");
  }

  struct Gem tempGem = _chosen_gems[0]._chosen_gem;

  //Chancing gems array index of _chosen_gems[0]
  gems[_chosen_gems[0]._chosen_gem.index_x][_chosen_gems[0]._chosen_gem.index_y] =
   _chosen_gems[1]._chosen_gem;
  

   //Chancing gems array index of _chosen_gems[1]
   gems[_chosen_gems[1]._chosen_gem.index_x][_chosen_gems[1]._chosen_gem.index_y] =
   tempGem;

   _update_gem_position_based_on_cell(_chosen_gems[0]._chosen_gem.index_x, _chosen_gems[0]._chosen_gem.index_y);
    _update_gem_position_based_on_cell(_chosen_gems[1]._chosen_gem.index_x, _chosen_gems[1]._chosen_gem.index_y);

   for(int i = 0; i < GEMCHOSENCOUNT; i++){
    _chosen_gems[i].gem_assigned = false;
   }
   

   printf("\n");
   for(int i = 0; i < ROWS; i++){
    for(int j = 0; j < COLS; j++){
      printf("%d", gems[i][j].type);
    }
    printf("\n");
  }
}

static bool _check_swap_gem_requirement(){
  //Check four direction
  return true; //Assuming it's true
}

/**
 * @brief Reset chosen gems
 * @note Only gem_assigned boolean reset, since _gem_choser() assumes
 * _chosen_gems.gems_chosen is not assigned if gem_assigned return false
 */
static void _reset_chosen_gems_state(){
  for(int i = 0; i < GEMCHOSENCOUNT; i++){
    _chosen_gems[i].gem_assigned = false;
  }
}

static void logic(){
if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      _gem_choser(_gem_return_on_mouse_click()); //Current it's assume player always clink into the grid
      if(_chosen_gems[GEMCHOSENCOUNT-1].gem_assigned == true){
        for(int i = 0; i < GEMCHOSENCOUNT; i++){
          printf("%d gem: %d\n", i, _chosen_gems[i]._chosen_gem.order);
        }
        if(_check_swap_gem_requirement()){
          _swap_gems();
          _reset_chosen_gems_state();
        }
      }
    }
}

int main(void) {
  _initialize_window();
  _initialize_game();

  while (!WindowShouldClose()) {
    logic();
    BeginDrawing();
    ClearBackground(_background_color);
    _draw_board();
    _draw_gems();
    EndDrawing();
  }

  return 0;
}
