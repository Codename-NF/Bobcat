#include <iostream>
#include "inputs.h"
#include <string>
#include <set> 
#include <list>
#include "structs.h"
#include "game.h"
#include "utils.h"
#include "constants.h"
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include "Graphics.h"

using namespace std;

void getting_names(battleship *game, int player_num) {
    
    bool all_players_joined = false;

    list<player>::iterator player_it = (*game).players.begin();
    int count = 0;
    while (!all_players_joined) {
// ******************* bluetooth place holder *******************
        string name = ask_for_name();
// ******************* bluetooth place holder *******************
        if (player_it->player_name.empty()) {
            player_it->player_name = name;
            player_it++;
            count++;
        }
        if (count == player_num) {
            all_players_joined = true;
        }
    }
}

void setting_up_ships(list<player>::iterator *p1, list<player>::iterator *p2, bool single_player_mode) {
    bool setup_finished = false;

    list<ship>::iterator shipsp1 = (*p1)->ships_list.begin();
    list<ship>::iterator shipsp2 = (*p2)->ships_list.begin();

    int p1count = 0;
    while (!setup_finished) {
// ******************* bluetooth place holder *******************
        setupvalues inputs = setupvalues(-1, -1, -1, -1, -1);
        if (p1count < NUM_OF_SHIPS) {
            inputs = ask_for_setup(1);
            p1count++;
        }
        else {
            if (!single_player_mode) {
                inputs = ask_for_setup(2);
            }
            else {
                cout << "Player 1 done setting up ships." << endl;
                return;
            }
        }
        int x_in = inputs.x;
        int y_in = inputs.y;
        int length = inputs.size;
        int orientation = inputs.orientation;
        bool came_from_player1;
        if (inputs.device_num == PLAYER1) {
            came_from_player1 = true;
        }
        else {
            came_from_player1 = false;
        }
// ******************* bluetooth place holder *******************
        if (out_of_bound(x_in, y_in, length, orientation)) {
            cout << "out of bound" << endl;
            p1count--;
            continue;
        }
        
        
        list<player>::iterator *currently_setting_up = came_from_player1 ? p1: p2;
        list<ship>::iterator *ships_being_set_up = came_from_player1 ? &shipsp1: &shipsp2;

        if (!path_empty(x_in, y_in, (*currently_setting_up)->all_boxes_on_board)) {
            cout << "another ship in the way for " << (*currently_setting_up)->player_name << endl;
            p1count--;
            continue;
        }
        
        // setting the ship
        if ((*ships_being_set_up)->size == 0) {
            (*ships_being_set_up)->orientation = orientation;
            (*ships_being_set_up)->size = length;
            (*ships_being_set_up)->start_box = box(x_in, y_in);
            (*ships_being_set_up)++;
        }

        int offset_x, offset_y;
        if (orientation == VERTICAL) {
            offset_x = 0;
            offset_y = 1;
        }
        else {
            offset_x = 1;
            offset_y = 0;
        }

        // adding all the boxes into the set
        for (int i = 0; i < length; i++) {
            (*currently_setting_up)->all_boxes_on_board.insert(box(x_in + (offset_x * i), y_in + (offset_y * i)));
        }

        if (shipsp1 == (*p1)->ships_list.end() && shipsp2 == (*p2)->ships_list.end()) {
            setup_finished = true;
            cout << "Both players finished setting up their ships" << endl;
            break;
        }
    }

}

void playing_game(list<player>::iterator *p1, list<player>::iterator *p2, bool single_player_mode) {
    bool game_finished = false;

    bool start1 = true;
    while (!game_finished) {
// ******************* bluetooth place holder *******************
        shootvalues inputs = shootvalues(-1, -1, -1);
        if (start1) {
            inputs = ask_for_shoot(1);
            start1 = !start1;
        }
        else {
            if (!single_player_mode) {
                inputs = ask_for_shoot(2);
                start1 = !start1;
            }
            else {
                // Get input from HARDWARE AI algorithm 

                // for now be random shoots
                /* initialize random seed: */
                cout << "AI to shoot......." << endl;
                srand (time(0));
                inputs.x = rand() % (BOUNDARY_MAX + 1);
                inputs.y = rand() % (BOUNDARY_MAX + 1);
                inputs.device_num = 2;

                cout << "AI shoots at (" << inputs.x << "," << inputs.y << ")" << endl;
                start1 = !start1;
            }
            
        }
        
        int x_in = inputs.x;
        int y_in = inputs.y;
        bool came_from_player1;
        if (inputs.device_num == 1) {
            came_from_player1 = true;
        }
        else {
            came_from_player1 = false;
        }
// ******************* bluetooth place holder *******************
        if (x_in < BOUNDARY_MIN || y_in < BOUNDARY_MIN || x_in > BOUNDARY_MAX || y_in > BOUNDARY_MAX) {
            cout << "Can't hit here buddy. Please try again." << endl;
            start1 = !start1;
            continue;
        }
        int current_attacking = came_from_player1 ? 1: 2;
        int next_up = came_from_player1 ? 2: 1;
        list<player>::iterator current_under_attack = came_from_player1 ? *p2: *p1;

        

        if (not_hit_yet(x_in, y_in, current_under_attack->boxes_hit)) {

            int status = check_hit_what(x_in, y_in, &(current_under_attack->ships_list), &(current_under_attack->remaining_ships), &(current_under_attack->ships_alive));

            current_under_attack->boxes_hit.insert(box(x_in, y_in, status));

            if (status == SUNK_STATUS_CODE) {
                // need to go through all boxes with that ship
                change_status_box_all_boxes(x_in, y_in, &(current_under_attack->boxes_hit), &(current_under_attack->ships_list));
            }

            cout << "Your hit status is " << status << endl;

        }
        else {
            cout << "Can't hit here buddy. Please try again." << endl;
            start1 = !start1;
            continue;
        }

        if (current_under_attack->remaining_ships == 0) {
            declare_win(current_attacking);
            game_finished = true;
        }
        else {
            cout << "Next up is player " << next_up << endl;
        }
    }

}


void AI_setting_up(list<player>::iterator *AI) {
    list<ship>::iterator ships_being_set_up = (*AI)->ships_list.begin();

    // setting up ships 
    // keep getting random place until it works? 
    // 5 ships, 2 3 3 4 5 
    int ship_sizes[5]= {2, 3, 3, 4, 5};

    int i = 0;
    while (i < NUM_OF_SHIPS) {
        srand (time(0));
        int x_in = rand() % (BOUNDARY_MAX + 1);
        int y_in = rand() % (BOUNDARY_MAX + 1);
        int orientation = (rand() % 2) + 1;
        int length = ship_sizes[i];
        if (out_of_bound(x_in, y_in, length, orientation)) {
            //cout << "out of bound" << endl;
            continue;
        }

        if (!path_empty(x_in, y_in, (*AI)->all_boxes_on_board)) {
            //cout << "another ship in the way for " << (*AI)->player_name << endl;
            continue;
        }

        // setting the ship
        if (ships_being_set_up->size == 0) {
            ships_being_set_up->orientation = orientation;
            ships_being_set_up->size = length;
            ships_being_set_up->start_box = box(x_in, y_in);
            ships_being_set_up++;
        }

        int offset_x, offset_y;
        if (orientation == VERTICAL) {
            offset_x = 0;
            offset_y = 1;
        }
        else {
            offset_x = 1;
            offset_y = 0;
        }

        // adding all the boxes into the set
        for (int i = 0; i < length; i++) {
            (*AI)->all_boxes_on_board.insert(box(x_in + (offset_x * i), y_in + (offset_y * i)));
        }

        i++;
        cout << "(" << x_in << " " << y_in << " " << length << " " << orientation << ")" << endl;
    }

}

createmessage create_lobby() {
    createmessage input1 = get_create_message_BT();

    while(input1.keywrod == 'f') {
        int input2 = get_join_message_BT();
        if (input2 == -1) {
            send_join_reponse_BT(FAILURE);
        }
        input1 = get_create_message_BT();
    }

    if (input1.keywrod == 'c') {
        send_create_response_BT(input1.numplayer, SUCCESS);
    }
    else {
        send_create_response_BT(input1.numplayer, FAILURE);
    }

    return input1;
}

int wait_for_player2() {
    int input2;

    do {
        input2 = get_join_message_BT();
    }
    while (input2 == -1);

    send_join_reponse_BT(SUCCESS);
    return input2;
}

void reject_player2() {
    int input2 = get_join_message_BT();
    if (input2 != -1) {
        send_join_reponse_BT(FAILURE);
    }
}

void setting_player_id(battleship *game, int player1_id) {
    list<player>::iterator player_it = (*game).players.begin();
    player_it->player_id = player1_id;

}
void setting_player_id(battleship *game, int player1_id, int player2_id) {
    list<player>::iterator player_it = (*game).players.begin();
    player_it->player_id = player1_id;
    player_it++;
    player_it->player_id = player2_id;
}

void assign_ship(list<player>::iterator *player, list<setupvalues>::iterator it) {
    list<ship>::iterator player_ships = (*player)->ships_list.begin();
    list<ship>::iterator *ships_being_set_up = &player_ships;

    int x_in = it->x;
    int y_in = it->y;
    int length = it->size;
    int orientation = it->orientation;
    bool came_from_player1;
    if (it->device_num == 1) {
        came_from_player1 = true;
    }
    else {
        came_from_player1 = false;
    }

    // setting the ship
    if ((*ships_being_set_up)->size == 0) {
        (*ships_being_set_up)->orientation = orientation;
        (*ships_being_set_up)->size = length;
        (*ships_being_set_up)->start_box = box(x_in, y_in);
        (*ships_being_set_up)++;
    }

    int offset_x, offset_y;
    if (orientation == VERTICAL) {
        offset_x = 0;
        offset_y = 1;
    }
    else {
        offset_x = 1;
        offset_y = 0;
    }

    // adding all the boxes into the set
    for (int i = 0; i < length; i++) {
        (*player)->all_boxes_on_board.insert(box(x_in + (offset_x * i), y_in + (offset_y * i)));
    }

}

void setting_up_ships_BT(list<player>::iterator *p1, list<player>::iterator *p2, bool single_player_mode) {
    list<setupvalues> list_of_placement;
    int err = 0;
    do {
        err = get_placement_message_BT(&list_of_placement, 1);
    }
    while (err);


    for (list<setupvalues>::iterator it = list_of_placement.begin(); it != list_of_placement.end(); it++) {
        assign_ship(p1, it);
    }

    if (single_player_mode) {
        AI_setting_up(p2);
        return;
    }

    do {
        err = get_placement_message_BT(&list_of_placement, 2);
    }
    while (err);
    
    for (list<setupvalues>::iterator it = list_of_placement.begin(); it != list_of_placement.end(); it++) {
        assign_ship(p2, it);
    }

}

void playing_game_BT(list<player>::iterator *p1, list<player>::iterator *p2, bool single_player_mode) {
    bool game_finished = false;

    bool turn_1 = true;
    send_game_start_status_BT(turn_1);

    while (!game_finished) {
        shootvalues inputs = shootvalues(-1, -1, -1);
        if (turn_1) {
            inputs = get_shoot_message_BT(1);
            turn_1 = false;
        }
        else {
            if (!single_player_mode) {
                inputs = get_shoot_message_BT(2);
                turn_1 = true;
            }
            else {
                // Get input from HARDWARE AI algorithm 
                // TODO 
                set<box> shots_with_ships;
                create_shots_with_ships(&((*p1)->boxes_hit), &shots_with_ships);
                //send_information_to_AI((*p1)->boxes_hit, (*p1)->ships_alive, shots_with_ships);
                int magic_number = 99;//some_input_function_from_AI();
                inputs.x = magic_number % 10;
                inputs.y = magic_number / 10;
                inputs.device_num = 2;
                turn_1 = true;
            }
            
        }

        // check for forfeit 
        // TODO: send to VGA as well 
        if (inputs.p1_forfeit) {
            game_finished = true;
            send_win_by_forfiet_BT(PLAYER2);
            break;
        }
        if (inputs.p2_forfeit) {
            game_finished = true;
            send_win_by_forfiet_BT(PLAYER1);
            break;
        }

        
        int x_in = inputs.x;
        int y_in = inputs.y;
        bool came_from_player1;
        if (inputs.device_num == 1) {
            came_from_player1 = true;
        }
        else {
            came_from_player1 = false;
        }
        
        int current_attacking = came_from_player1 ? 1: 2;
        int next_up = came_from_player1 ? 2: 1;
        list<player>::iterator current_under_attack = came_from_player1 ? *p2: *p1;
        int status;
        
        if (not_hit_yet(x_in, y_in, current_under_attack->boxes_hit)) {

            status = check_hit_what(x_in, y_in, &(current_under_attack->ships_list), &(current_under_attack->remaining_ships), &(current_under_attack->ships_alive));

            current_under_attack->boxes_hit.insert(box(x_in, y_in, status));

            if (status == SUNK_STATUS_CODE) {
                // need to go through all boxes with that ship
                ship sunk_ship = change_status_box_all_boxes(x_in, y_in, &(current_under_attack->boxes_hit), &(current_under_attack->ships_list));

                // only if it's sunk it would be a possibly of gameover 
                if (current_under_attack->remaining_ships == 0) {
                    game_finished = true;
                }
                send_result_message_BT(current_attacking, x_in, y_in, game_finished, status, sunk_ship.start_box.x, sunk_ship.start_box.y, sunk_ship.size, sunk_ship.orientation);
                send_targeted_message_BT(next_up, x_in, y_in, game_finished, status, sunk_ship.start_box.x, sunk_ship.start_box.y, sunk_ship.size, sunk_ship.orientation);
                squaremapper(x_in, y_in, next_up, HIT_COLOR);
                squaremappership(next_up, sunk_ship.start_box.x, sunk_ship.start_box.y, sunk_ship.size, sunk_ship.orientation, game_finished, SUNK_CROSS_COLOR);
                continue;
            }
            send_result_message_BT(current_attacking, x_in, y_in, game_finished, status);
            send_targeted_message_BT(next_up, x_in, y_in, game_finished, status);
            if (status == HIT_STATUS_CODE) {
                squaremapper(x_in, y_in, next_up, HIT_COLOR);
            }
            else {
                squaremapper(x_in, y_in, next_up, MISS_COLOR);
            }
            

        }

        
    }
}
