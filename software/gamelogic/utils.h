#include <iostream>
#include <list>
#include <set>
#include <string>
#include <sys/socket.h>
#include "constants.h"
#include "structs.h"

void declare_win (int player_who_won);
bool out_of_bound(int x_start, int y_start, int size, int orientation); 
bool path_empty(int x_start, int y_start, set<box> all_boxes_on_board);
bool contains_box(ship *ship, int x, int y);
int check_hit_what(int x, int y, list<ship> *ships, int *remaining_ships);
bool not_hit_yet(int x, int y, set<box> boxes);
