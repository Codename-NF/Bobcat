#include "inputs.h"
#include "constants.h"
#include "structs.h"
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string>
#include <bitset>
#include "RS232.h" // may need to change path 
#include "ai_firm.h"

using namespace std;

// ******************* bluetooth place holder *******************
string ask_for_name() {
    string name; 
    cout << "Please type your name: "; // Type a number and press enter
    cin >> name; // Get user input from the keyboard
    cout << "Your name is: " << name << endl; // Display the input value

    return name;
}

setupvalues ask_for_setup(int player_num) {
    int x;
    int y;
    int size;
    int orientation;
    int device_num;

    cout << "Player " << player_num << endl;
    cout << "What x position would you like to place your ship: ";
    cin >> x;
    cout << "What y position would you like to place your ship: ";
    cin >> y;
    cout << "What size is the ship: ";
    cin >> size;
    cout << "What orientation is the ship (1 for vertical and 2 for horizntaal): ";
    cin >> orientation;

    device_num = player_num;

    return setupvalues(x, y, size, orientation, device_num);
}

shootvalues ask_for_shoot(int player_num) {
    int x;
    int y;
    int device_num;

    cout << "Player " << player_num << endl;
    cout << "What x position would you like to shoot: ";
    cin >> x;
    cout << "What y position would you liek to shoot: ";
    cin >> y;

    device_num = player_num;

    return shootvalues(x, y, device_num);

}

// ******************* bluetooth place holder *******************
// taking in a list of inputs 
/*
Format: 
Placement message (app to console)
“p coordinate1 bitfield1 coordinate2 bitfield2 ...~” 
Note: These are shown with spaces in between, but they are not sent in reality
coordinate is a byte taking on values from 0 to 99 inclusive, where the coordinate of the ship is actually at (coordinate % 10, coordinate / 10)
bitfield contains two attributes
Upper 4 bits are unused
shipOrientation (0/1) (1 bit) -> 0 is vertical and 1 is horizontal 
shipLength (2 - 5) (3 bits)
Bit 0 is shipOrientation, bit 1-3 are shipLength
1011 is a 5 length ship that is horizontal
0100 is a 2 length ship that is vertical

*/
int get_placement_message_BT(list<setupvalues> *list_setupval, int device_num) {
    char receive_char[BT_RECEIVE_SIZE];
    if (device_num == 1) {
        BT_receive_0(receive_char);
    }
    else {
        BT_receive_1(receive_char);
    }

    if (receive_char[0] != 'p') {
        return FAILURE;
    }
    // The start of placement message is at 1, 3, 5, 7, 9
    for (int i = 1; i < 10; i += 2) {
        int x, y, size, orientation;
        x = receive_char[i] % 10;
        y = receive_char[i] / 10;
        char bitfield = receive_char[i+1];
        orientation = (bitfield & 1) + 1; // get the last bit of the bitfield 
        size = bitfield >> 1; // getting rid of the last bit of the bitfield

        (*list_setupval).push_back(setupvalues(x, y, size, orientation, device_num));

        cout << "(" << x << ", " << y << ")" << "size: " << size << ", orientation: " << orientation << endl;

    }
    return SUCCESS;
}

/*
Format:
“s xCoordinate yCoordinate~”
*/
int get_shoot_message_BT(shootvalues *input, int device_num) {
    char receive_char[BT_RECEIVE_SIZE];
    char forfeit_char[BT_RECEIVE_SIZE];
    int first_success = 0;
    int second_success = 0;
    if (device_num == 1) {
        first_success = BT_receive_0(receive_char);
        second_success = BT_receive_1(forfeit_char);
    }
    else {
        first_success = BT_receive_1(receive_char);
        second_success = BT_receive_0(forfeit_char);
    }

    if (first_success == 0) {
        return FAILURE;
    }

    // checking for forfeit messages at the beginning of each turn 
    if (receive_char[0] == 'f') {
        if (device_num == 1) {
            (*input).p1_forfeit = true;
            return SUCCESS;
        }
        (*input).p2_forfeit = true;
        return SUCCESS;
    }

    if (second_success && forfeit_char[0] == 'f') {
        if (device_num == 1) {
            (*input).p2_forfeit = true;
            return SUCCESS;
        }
        (*input).p1_forfeit = true;
        return SUCCESS;
    }

    // checking if it's the correct shoot message
    if (receive_char[0] != 's') {
        return FAILURE;
    }

    (*input).x = receive_char[2] - '0'; // subtract char 0 becasue we want the int value of the char instead of the asci decimal value of the char
    (*input).y = receive_char[4] - '0';
    (*input).device_num = device_num;

    return SUCCESS;
}

/*
Format:
“start isTurn~”
isTurn is 1 if the current player is attacking first, 0 otherwise
*/
void send_game_start_status_BT(bool start1, bool single_player_mode) {
    stringstream message_to_1;
    message_to_1 << "start " << start1 << "~";
    BT_send_0(message_to_1.str().c_str());
    if (!single_player_mode) {
        stringstream message_to_2;
        message_to_2 << "start " << !start1 << "~";
        BT_send_1(message_to_2.str().c_str());
    }
}

/*
Format: (the result for attacking)
“result xCoordinate yCoordinate gamestatus hitstatus~”
*/
void send_result_message_BT(int device_num, int x, int y, int gamestatus, int hitstatus, bool single_player_mode) {
    stringstream message;
    message << "result " << x << " " << y << " " << gamestatus << " " << hitstatus << "~";

    if (device_num == 1) {
        BT_send_0(message.str().c_str());
    }
    else if(!single_player_mode){
        BT_send_1(message.str().c_str());
    }

}

/*
Format: (the result for attacking)
“targeted xCoordinate yCoordinate gameStatus hitstatus (destroyedShipXCoordinate destroyedShipYCoordinate shipLength shipOrientation)~”
*/
void send_result_message_BT(int device_num, int x, int y, int gamestatus, int hitstatus, int destroyed_start_x, int destroyed_start_y, int length, int orientation, bool single_player_mode) {
    stringstream message;
    message << "result " << x << " " << y << " " << gamestatus << " " << hitstatus << " " << destroyed_start_x << " " << destroyed_start_y << " " << length << " " << orientation << "~";

    if (device_num == 1) {
        BT_send_0(message.str().c_str());
    }
    else if(!single_player_mode){
        BT_send_1(message.str().c_str());
    }

}

/*
Format: (the result of being attacked)
“targeted xCoordinate yCoordinate gameStatus hitstatus (destroyedShipXCoordinate destroyedShipYCoordinate shipLength shipOrientation)~”
*/
void send_targeted_message_BT(int device_num, int x, int y, int gamestatus, int hitstatus, bool single_player_mode) {
    stringstream message;
    message << "targeted " << x << " " << y << " " << gamestatus << " " << hitstatus << "~";

    if (device_num == 1) {
        BT_send_0(message.str().c_str());
    }
    else if(!single_player_mode){
        BT_send_1(message.str().c_str());
    }

}

/*
Format: (the result of being attacked)
“targeted xCoordinate yCoordinate gameStatus hitstatus (destroyedShipXCoordinate destroyedShipYCoordinate shipLength shipOrientation)~”
*/
void send_targeted_message_BT(int device_num, int x, int y, int gamestatus, int hitstatus, int destroyed_start_x, int destroyed_start_y, int length, int orientation, bool single_player_mode) {
    stringstream message;
    message << "targeted " << x << " " << y << " " << gamestatus << " " << hitstatus << " " << destroyed_start_x << " " << destroyed_start_y << " " << length << " " << orientation << "~";

    if (device_num == 1) {
        BT_send_0(message.str().c_str());
    }
    else if(!single_player_mode){
        BT_send_1(message.str().c_str());
    }

}


/*
Format:
Create message (app to console)
“c mode playerID ~” 
playerID is the ID of the player
Mode is 0 or 1 or 2
    0  for easy AI mode, 
    1 for hard AI mode,
    2 for multiplayer

Only sent to BluetoothChip0

*/
int get_create_message_BT(createmessage *msg) {
    char receive_char[BT_RECEIVE_SIZE];

    
    if (BT_receive_0(receive_char) == SUCCESS) {
        if (receive_char[0] != 'c') { 
            return FAILURE;
        } 
        (*msg).keywrod = receive_char[0];
        (*msg).playing_mode = receive_char[2] - '0'; // subtract char 0 becasue we want the int value of the char instead of the asci decimal value of the char
        (*msg).playerid = atoi(receive_char + 4); // get the rest of the string and read that as int 
        
        return SUCCESS;
    }
    return FAILURE;
    
}

/*
Format:
“create mode status~”
Mode is 0 or 1 or 2
    0  for easy AI mode, 
    1 for hard AI mode,
    2 for multiplayer
status is “1” or “0”
“0” if a game is already in progress
Else “1” if multiplayer lobby created successfully
*/
void send_create_response_BT(int mode, int status) {
    stringstream message;
    message << "create " << mode << " " << status << "~";
    BT_send_0(message.str().c_str());
    
}


/*
Format:
Join message (app to console)
“j playerID~”
playerID is the player ID
Can only join a pre-existing lobby
Only sent to BluetoothChip1
returns player id on success 
-1 on failsure
*/
int get_join_message_BT() {
    char receive_char[BT_RECEIVE_SIZE];

    
    if (BT_receive_1(receive_char) == SUCCESS) {
        if (receive_char[0] == 'j') {
            return atoi(receive_char + 2);
        }
    }
    return -1;
    
}

void send_join_reponse_BT(int status) {
    stringstream message;
    message << "join " << status << "~";
    BT_send_1(message.str().c_str());

}


/*
Format:
“ready~”
Only sent for multiplayer lobbies
Sent when the multiplayer lobby is full and the players may begin placement
*/

void send_ready_message_BT() {
    BT_send_0("ready~");
    BT_send_1("ready~");
}


/*
Format:
"f~" 
to indicate that the the other player have forfeited 
*/
void send_win_by_forfeit_BT(int device_num) {
    if (device_num == 1) {
        BT_send_0("f~");
    }
    else {
        BT_send_1("f~");
    }
}

int where_to_shoot_AI(set<box> fired, bitset<5> ships_alive, set<box> hits) {
    bitset<100> fired_bits = 0;
    bitset<100> hits_bits = 0;

    for(set<box>::iterator it = fired.begin(); it != fired.end(); it++) {
        int number = (it->y)*10 + it->x;
        fired_bits[number] = 1;
    }

    for(set<box>::iterator it = hits.begin(); it != hits.end(); it++) {
        int number = (it->y)*10 + it->x;
        hits_bits[number] = 1;
    }

    // divider to split up the 100 bits to 32 bits fragments that the algorithm can take
    bitset<100> divider (std::string("0000000000000000000000000000000000000000000000000000000000000000000011111111111111111111111111111111"));

    // bit shift and AND operations to get the right bits 
    unsigned long fired0 = (fired_bits & divider).to_ulong();
    unsigned long fired1 = ((fired_bits >> 32) & divider).to_ulong();
    unsigned long fired2 = ((fired_bits >> 64) & divider).to_ulong();
    unsigned long fired3 = ((fired_bits >> 96) & divider).to_ulong();

    unsigned long hits0 = (hits_bits & divider).to_ulong();
    unsigned long hits1 = ((hits_bits >> 32) & divider).to_ulong();
    unsigned long hits2 = ((hits_bits >> 64) & divider).to_ulong();
    unsigned long hits3 = ((hits_bits >> 96) & divider).to_ulong();

    int alive_ships = (int) (ships_alive.to_ulong());

    return ai_where_to_shoot(fired0, fired1, fired2, fired3, hits0, hits1, hits2, hits3, alive_ships);
    
}
