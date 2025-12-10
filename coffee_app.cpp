#include "coffee_app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // For strcasecmp

// --- Theme Colors ---
#define COL_BG          lv_color_hex(0x101024) // Dark Blue/Black
#define COL_CARD        lv_color_hex(0x1E1E38) // Card BG
#define COL_ACCENT      lv_color_hex(0x00D2FF) // Neon Cyan
#define COL_TEXT        lv_color_hex(0xFFFFFF) // White
#define COL_TEXT_SEC    lv_color_hex(0x8888AA) // Grey
#define COL_SUCCESS     lv_color_hex(0x00FF88) // Green
#define COL_WARNING     lv_color_hex(0xFFAA00) // Orange
#define COL_GAME_BG     lv_color_hex(0x2A2A4A) 
#define COL_GAME_X      lv_color_hex(0xFF4444) 
#define COL_GAME_O      lv_color_hex(0x4444FF) 
#define COL_BEAN        lv_color_hex(0x8B4513) 
#define COL_CATCHER     lv_color_hex(0xCD853F) 
#define COL_GOLD        lv_color_hex(0xFFD700) 

// --- Liquid Colors (Gradients) ---
#define COL_LIQ_COFFEE_DARK lv_color_hex(0x3B2F2F)
#define COL_LIQ_COFFEE_LITE lv_color_hex(0x6F4E37)
#define COL_LIQ_MILK_DARK   lv_color_hex(0xDCDCDC)
#define COL_LIQ_MILK_LITE   lv_color_hex(0xFFFFFF)
#define COL_LIQ_FOAM_DARK   lv_color_hex(0xDEB887)
#define COL_LIQ_FOAM_LITE   lv_color_hex(0xFFE4B5)
#define COL_LIQ_CHOCO_DARK  lv_color_hex(0x5D4037)
#define COL_LIQ_CHOCO_LITE  lv_color_hex(0x8D6E63)

// --- IMAGE DECLARATIONS ---
LV_IMG_DECLARE(img_Espresso);
LV_IMG_DECLARE(img_Americano);
LV_IMG_DECLARE(img_Latte);
LV_IMG_DECLARE(img_Irish_Coffee); 

// --- Data ---
// Reordered: Make it yours is first (Index 0)
const char* coffee_names[] = {"Make it yours", "Espresso", "Americano", "Latte"};
const char* coffee_prices[] = {"6.00", "2.99", "3.99", "4.99"};

const lv_img_dsc_t* coffee_images[] = {
    &img_Irish_Coffee, // Visual for Make it yours
    &img_Espresso,
    &img_Americano,
    &img_Latte
};

const uint16_t coffee_zooms[] = { 280, 360, 360, 280 };

// --- Recipe Memory ---
CustomRecipe saved_recipes[10];
int recipe_count = 0;

// --- State ---
int selected_coffee_index = 0;
int current_cloud_order_id = -1; // Track Cloud Order ID
float final_price = 0.0;
bool is_logged_in = false; 
lv_timer_t * mood_timer = NULL; // Timer for hiding the mood label

// --- UI Objects ---
lv_obj_t * brew_bar = NULL;
lv_obj_t * brew_label = NULL;
lv_obj_t * btn_done_playing = NULL;
lv_obj_t * slider_size = NULL; // Global reference for size slider

// Customization Objects
lv_obj_t * glass_obj = NULL; // Reference for collision detection
lv_obj_t * liq_coffee = NULL;
lv_obj_t * liq_milk = NULL;
lv_obj_t * liq_foam = NULL;
lv_obj_t * liq_choco = NULL;
lv_obj_t * lbl_mood = NULL;
lv_obj_t * ta_recipe_name = NULL;
lv_obj_t * btn_save_recipe = NULL;
lv_obj_t * kb_recipe = NULL;
lv_obj_t * right_panel_miy = NULL; // Make it yours panel

// New Controls for Make it Yours
lv_obj_t * dd_coffee_type = NULL;
lv_obj_t * dd_milk_type = NULL;

#define MAX_GLASS_HEIGHT 220 // Max total height of liquids

// Drag Contexts
struct DragContext {
    int origin_x;
    int origin_y;
    lv_obj_t** target_liq_ptr; 
    lv_obj_t* dropdown_ref; // Reference to dropdown to read value
};
DragContext dc_coffee, dc_milk, dc_foam, dc_choco;

// Payment/Game Objects
lv_obj_t * kb_input = NULL;
lv_obj_t * ta_login = NULL;
lv_obj_t * ta_promo = NULL;
lv_obj_t * panel_payment = NULL;
lv_obj_t * lbl_total_pay = NULL;
lv_obj_t * btn_login_main = NULL;

// Game Vars
lv_obj_t * ttt_container = NULL;
lv_obj_t * ttt_btns[9];
int ttt_board[9]; 
bool game_active = true;
int ttt_discount = 0;
lv_obj_t * bean_container = NULL;
lv_obj_t * catcher_obj = NULL;
lv_obj_t * lbl_score = NULL;
lv_timer_t * bean_timer = NULL;
lv_timer_t * brew_timer = NULL;
#define MAX_BEANS 10
#define BEAN_SIZE 20
#define CATCHER_W 60
#define CATCHER_H 30
#define GAME_AREA_H 200  
#define GAME_AREA_W 280
struct FallingBean { lv_obj_t * obj; bool active; int x, y; };
FallingBean beans[MAX_BEANS];
int beans_caught = 0;
int beans_spawned = 0;
int current_game_mode = 0;

// --- Styles ---
static lv_style_t style_screen;
static lv_style_t style_card;
static lv_style_t style_title;
static lv_style_t style_btn;
static lv_style_t style_btn_circle;
static lv_style_t style_btn_green;
static lv_style_t style_slider_main;
static lv_style_t style_slider_indicator;
static lv_style_t style_slider_knob;
static lv_style_t style_game_btn;
static lv_style_t style_coupon;

// New Liquid Styles
static lv_style_t style_glass;
static lv_style_t style_liq_coffee;
static lv_style_t style_liq_milk;
static lv_style_t style_liq_foam;
static lv_style_t style_liq_choco;

// --- Forward Declarations ---
void create_splash_screen();
void create_menu_screen();
void create_customize_screen();
void create_make_it_yours_screen(); 
void create_payment_screen();
void create_game_screen();
void create_enjoy_screen();
void create_pickup_screen(); // New Screen

// --- Helper: Screen Switcher ---
void switch_to_screen(lv_obj_t * new_scr) {
    lv_obj_t * old_scr = lv_scr_act();
    lv_scr_load(new_scr);
    if(old_scr) lv_obj_del(old_scr);
}

// --- Callbacks ---
static void btn_start_cb(lv_event_t * e) { create_menu_screen(); }
static void btn_pickup_cb(lv_event_t * e) { create_pickup_screen(); } // New Callback

static void btn_coffee_select_cb(lv_event_t * e) {
    selected_coffee_index = (int)(intptr_t)lv_event_get_user_data(e);
    final_price = atof(coffee_prices[selected_coffee_index]);
    
    // Index 0 is now "Make it yours"
    if(selected_coffee_index == 0) {
        create_make_it_yours_screen();
    } else {
        create_customize_screen();
    }
}
static void btn_back_to_menu_cb(lv_event_t * e) { create_menu_screen(); }
static void btn_back_to_customize_cb(lv_event_t * e) { 
    if(selected_coffee_index == 0) create_make_it_yours_screen();
    else create_customize_screen(); 
}
static void btn_checkout_cb(lv_event_t * e) { 
    // Add cost for size if applicable
    if(selected_coffee_index > 0 && slider_size) {
        int val = lv_slider_get_value(slider_size);
        if(val == 1) final_price += 0.50; // Medium
        if(val == 2) final_price += 1.00; // Large
    }
    create_payment_screen(); 
}
static void btn_pay_cb(lv_event_t * e) { 
    // Update Cloud Status if this is a cloud order
    if(current_cloud_order_id != -1) {
        update_order_status_cloud(current_cloud_order_id, "brewing");
    }
    create_game_screen(); 
}
static void btn_home_cb(lv_event_t * e) { create_splash_screen(); }
static void btn_done_playing_cb(lv_event_t * e) { create_enjoy_screen(); }

// --- Customization Logic (Make it Yours) ---

static int get_total_liquid_height() {
    int h = 0;
    if(liq_coffee) h += lv_obj_get_height(liq_coffee);
    if(liq_milk) h += lv_obj_get_height(liq_milk);
    if(liq_foam) h += lv_obj_get_height(liq_foam);
    if(liq_choco) h += lv_obj_get_height(liq_choco);
    return h;
}

// Timer callback to hide the mood label
static void hide_mood_cb(lv_timer_t * t) {
    if(lbl_mood) lv_obj_add_flag(lbl_mood, LV_OBJ_FLAG_HIDDEN);
    mood_timer = NULL; 
}

// Cleanup callback when label is deleted (screen change)
static void lbl_mood_delete_cb(lv_event_t * e) {
    if(mood_timer) {
        lv_timer_del(mood_timer);
        mood_timer = NULL;
    }
    lbl_mood = NULL;
}

static void update_mood_label() {
    int v_coffee = lv_obj_get_height(liq_coffee);
    int v_milk = lv_obj_get_height(liq_milk);
    int v_foam = lv_obj_get_height(liq_foam);
    int v_choco = lv_obj_get_height(liq_choco);

    const char* mood = "The Alchemist";
    if (v_choco > 40) mood = "Sweet Tooth";
    else if (v_coffee > 80 && v_milk < 30) mood = "Midnight Oil";
    else if (v_milk > 70 && v_foam > 50) mood = "Cloud Nine";
    else if (v_milk > 60 && v_coffee < 40) mood = "Velvet Hug";
    else if (v_coffee > 60 && v_milk > 60) mood = "Power Balance";
    else if (v_foam > 80) mood = "Foam Party";
    
    if(lbl_mood) {
        lv_label_set_text(lbl_mood, mood);
        
        // "Pop" logic: Show label, bring to front, reset timer
        lv_obj_clear_flag(lbl_mood, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(lbl_mood);
        
        if(mood_timer) {
            lv_timer_reset(mood_timer);
            lv_timer_resume(mood_timer);
        } else {
            mood_timer = lv_timer_create(hide_mood_cb, 2000, NULL);
            lv_timer_set_repeat_count(mood_timer, 1);
        }
    }
}

// Drag liquid inside glass to resize OR remove
static void layer_drag_event_cb(lv_event_t * e) {
    lv_obj_t * liq = lv_event_get_target(e);
    lv_obj_t * source_container = (lv_obj_t *)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);
    lv_indev_t * indev = lv_indev_get_act();
    if(indev == NULL) return;
    
    if(code == LV_EVENT_PRESSING) {
        // Vertical Drag: Resize
        lv_point_t vect;
        lv_indev_get_vect(indev, &vect);
        
        int current_h = lv_obj_get_height(liq);
        int total_h = get_total_liquid_height();
        int other_h = total_h - current_h;
        int max_allowed = MAX_GLASS_HEIGHT - other_h;

        int32_t new_h = current_h - vect.y; 
        
        if(new_h < 0) new_h = 0;
        if(new_h > max_allowed) new_h = max_allowed;
        
        lv_obj_set_height(liq, new_h);
        update_mood_label();
    }
    else if(code == LV_EVENT_RELEASED) {
        // Check if dragged OUT of glass (Horizontal check)
        lv_point_t point;
        lv_indev_get_point(indev, &point);
        
        lv_area_t glass_area;
        lv_obj_get_coords(glass_obj, &glass_area);
        
        // If X coordinate is outside the glass range, remove it
        if(point.x < glass_area.x1 || point.x > glass_area.x2) {
            lv_obj_set_height(liq, 0); // Remove liquid
            if(source_container) lv_obj_clear_flag(source_container, LV_OBJ_FLAG_HIDDEN); // Show source container again
            update_mood_label();
        }
    }
}

// Drag Source Ingredient to Glass
static void source_drag_event_cb(lv_event_t * e) {
    lv_obj_t * obj = lv_event_get_target(e); // This is the container now
    DragContext * dc = (DragContext *)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_PRESSING) {
        lv_indev_t * indev = lv_indev_get_act();
        if(indev) {
            lv_point_t vect;
            lv_indev_get_vect(indev, &vect);
            lv_coord_t x = lv_obj_get_x(obj) + vect.x;
            lv_coord_t y = lv_obj_get_y(obj) + vect.y;
            lv_obj_set_pos(obj, x, y);
        }
    }
    else if(code == LV_EVENT_RELEASED) {
        // Check collision with glass
        lv_area_t glass_area;
        lv_obj_get_coords(glass_obj, &glass_area);
        lv_area_t obj_area;
        lv_obj_get_coords(obj, &obj_area);

        if(_lv_area_is_on(&glass_area, &obj_area)) {
            // Hit! Activate the liquid layer
            if(*(dc->target_liq_ptr)) {
                int total_h = get_total_liquid_height();
                int available = MAX_GLASS_HEIGHT - total_h;
                
                if(available > 20) { // Only add if there is meaningful space
                    int fill_amt = (available > 50) ? 50 : available;
                    lv_obj_t * liq = *(dc->target_liq_ptr);
                    
                    // --- Apply Custom Properties based on Dropdowns ---
                    if(liq == liq_coffee && dc->dropdown_ref) {
                        char buf[32];
                        lv_dropdown_get_selected_str(dc->dropdown_ref, buf, sizeof(buf));
                        // Format: "Hot Light", "Cold Dark", etc.
                        lv_color_t c = COL_LIQ_COFFEE_LITE;
                        if(strstr(buf, "Dark")) c = COL_LIQ_COFFEE_DARK;
                        else if(strstr(buf, "Light")) c = lv_color_hex(0x8B5A2B); // Lighter brown
                        
                        // Temperature tint (Cold = Blueish)
                        if(strstr(buf, "Cold")) c = lv_color_mix(c, lv_color_hex(0x0000FF), 230);
                        
                        lv_obj_set_style_bg_color(liq, c, 0);
                        lv_obj_set_style_bg_grad_color(liq, lv_color_darken(c, 20), 0);
                    }
                    else if(liq == liq_milk && dc->dropdown_ref) {
                        int id = lv_dropdown_get_selected(dc->dropdown_ref);
                        lv_color_t c = COL_LIQ_MILK_LITE;
                        if(id == 1) c = lv_color_hex(0xE0D5C0); // Oat (Beige)
                        else if(id == 2) c = lv_color_hex(0xFFEBCD); // Almond (Off-white)
                        
                        lv_obj_set_style_bg_color(liq, c, 0);
                        lv_obj_set_style_bg_grad_color(liq, lv_color_darken(c, 10), 0);
                    }

                    lv_obj_set_height(liq, fill_amt); 
                    
                    // --- STACKING LOGIC ---
                    // Move the newly added liquid to index 0.
                    // In a COLUMN layout with ALIGN_END, index 0 is at the top of the stack visually.
                    lv_obj_move_to_index(liq, 0);

                    // Hide the entire source container (Icon + Dropdown)
                    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN); 
                    update_mood_label();
                }
            }
        }
        
        // Snap back to original position (hidden or not)
        lv_obj_set_pos(obj, dc->origin_x, dc->origin_y);
    }
}

static void btn_save_recipe_cb(lv_event_t * e) {
    const char* name = lv_textarea_get_text(ta_recipe_name);
    if(strlen(name) == 0) return;

    if(recipe_count < 10) {
        strcpy(saved_recipes[recipe_count].name, name);
        saved_recipes[recipe_count].coffee_val = lv_obj_get_height(liq_coffee);
        saved_recipes[recipe_count].milk_val = lv_obj_get_height(liq_milk);
        saved_recipes[recipe_count].foam_val = lv_obj_get_height(liq_foam);
        recipe_count++;
        
        lv_obj_t * mbox = lv_msgbox_create(NULL, "Saved!", "Your recipe is safe.", NULL, true);
        lv_obj_center(mbox);
    }
}

static void kb_recipe_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        const char* txt = lv_textarea_get_text(ta_recipe_name);
        lv_obj_add_flag(kb_recipe, LV_OBJ_FLAG_HIDDEN);
        
        // Move panel back down
        if(right_panel_miy) lv_obj_set_y(right_panel_miy, -10);

        // Search for recipe
        for(int i=0; i<recipe_count; i++) {
            if(strcmp(saved_recipes[i].name, txt) == 0) {
                // Load it!
                lv_obj_set_height(liq_coffee, saved_recipes[i].coffee_val);
                lv_obj_set_height(liq_milk, saved_recipes[i].milk_val);
                lv_obj_set_height(liq_foam, saved_recipes[i].foam_val);
                update_mood_label();
                
                char buf[64];
                sprintf(buf, "Welcome back!\nBrewing %s...", saved_recipes[i].name);
                lv_obj_t * mbox = lv_msgbox_create(NULL, "Recipe Loaded", buf, NULL, true);
                lv_obj_center(mbox);
                break;
            }
        }
    }
}

static void ta_recipe_focus_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_FOCUSED) {
        lv_obj_clear_flag(kb_recipe, LV_OBJ_FLAG_HIDDEN);
        // Move panel up so keyboard doesn't cover text area
        if(right_panel_miy) lv_obj_set_y(right_panel_miy, -120);
    }
    else if(code == LV_EVENT_DEFOCUSED) {
        lv_obj_add_flag(kb_recipe, LV_OBJ_FLAG_HIDDEN);
        if(right_panel_miy) lv_obj_set_y(right_panel_miy, -10);
    }
}

// --- Payment Logic ---
static void update_price_label() {
    char buf[32];
    sprintf(buf, "Total: $%.2f", final_price);
    lv_label_set_text(lbl_total_pay, buf);
}

static void kb_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        lv_obj_add_flag(kb_input, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ta_login, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ta_promo, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(panel_payment, LV_OBJ_FLAG_HIDDEN);
        
        if(lv_keyboard_get_textarea(kb_input) == ta_promo) {
            const char* txt = lv_textarea_get_text(ta_promo);
            if(strlen(txt) > 0) {
                final_price = final_price * 0.95; 
                update_price_label();
                lv_obj_t * mbox = lv_msgbox_create(NULL, "Promo Applied", "5% Discount Applied!", NULL, true);
                lv_obj_center(mbox);
            }
        }
        else if(lv_keyboard_get_textarea(kb_input) == ta_login) {
             const char* txt = lv_textarea_get_text(ta_login);
             if(strlen(txt) > 0) {
                is_logged_in = true;
                lv_label_set_text(lv_obj_get_child(btn_login_main, 0), "Member Logged In");
                lv_obj_set_style_bg_color(btn_login_main, COL_SUCCESS, 0);
                lv_label_set_text(lv_obj_get_child(btn_login_main, 0), "Enter Promo Code");
             }
        }
    }
}

static void btn_login_toggle_cb(lv_event_t * e) {
    lv_obj_add_flag(panel_payment, LV_OBJ_FLAG_HIDDEN);
    if(!is_logged_in) {
        lv_obj_clear_flag(ta_login, LV_OBJ_FLAG_HIDDEN);
        lv_keyboard_set_textarea(kb_input, ta_login);
        lv_keyboard_set_mode(kb_input, LV_KEYBOARD_MODE_NUMBER);
    } else {
        lv_obj_clear_flag(ta_promo, LV_OBJ_FLAG_HIDDEN);
        lv_keyboard_set_textarea(kb_input, ta_promo);
        lv_keyboard_set_mode(kb_input, LV_KEYBOARD_MODE_TEXT_UPPER);
    }
    lv_obj_clear_flag(kb_input, LV_OBJ_FLAG_HIDDEN);
}

// --- Game Logic (Tic Tac Toe + Beans) ---
static void switch_to_beans_cb(lv_timer_t * t) {
    lv_obj_add_flag(ttt_container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(bean_container, LV_OBJ_FLAG_HIDDEN);
    current_game_mode = 1;
    lv_timer_del(t);
}

static void check_win() {
    bool over = false;
    const char* msg = "";
    int wins[8][3] = {{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}};
    for(int i=0; i<8; i++) {
        if(ttt_board[wins[i][0]] != 0 && 
           ttt_board[wins[i][0]] == ttt_board[wins[i][1]] && 
           ttt_board[wins[i][0]] == ttt_board[wins[i][2]]) {
            game_active = false;
            if(ttt_board[wins[i][0]] == 1) { msg = "You Win!"; ttt_discount = 10; } 
            else { msg = "AI Wins!"; ttt_discount = 5; }
            over = true;
            break;
        }
    }
    if(!over) {
        bool full = true;
        for(int i=0; i<9; i++) if(ttt_board[i] == 0) full = false;
        if(full) { game_active = false; msg = "Draw!"; ttt_discount = 5; over = true; }
    }
    if(over) {
        lv_label_set_text(brew_label, msg);
        lv_timer_create(switch_to_beans_cb, 1500, NULL);
    }
}

static int get_winning_move(int player) {
    int wins[8][3] = {{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}};
    for(int i=0; i<8; i++) {
        int p_count = 0;
        int empty_idx = -1;
        for(int j=0; j<3; j++) {
            if(ttt_board[wins[i][j]] == player) p_count++;
            else if(ttt_board[wins[i][j]] == 0) empty_idx = wins[i][j];
        }
        if(p_count == 2 && empty_idx != -1) return empty_idx;
    }
    return -1;
}

static void ai_move() {
    if(!game_active) return;
    int move = -1;
    move = get_winning_move(2);
    if(move == -1) move = get_winning_move(1);
    if(move == -1 && ttt_board[4] == 0) move = 4;
    if(move == -1) {
        int available[9];
        int count = 0;
        for(int i=0; i<9; i++) if(ttt_board[i] == 0) available[count++] = i;
        if(count > 0) move = available[rand() % count];
    }
    if(move != -1) {
        ttt_board[move] = 2;
        lv_obj_set_style_bg_color(ttt_btns[move], COL_GAME_O, 0);
        lv_label_set_text(lv_obj_get_child(ttt_btns[move], 0), "O");
        check_win();
    }
}

static void ttt_btn_cb(lv_event_t * e) {
    if(!game_active) return;
    int idx = (int)(intptr_t)lv_event_get_user_data(e);
    if(ttt_board[idx] == 0) {
        ttt_board[idx] = 1;
        lv_obj_set_style_bg_color(ttt_btns[idx], COL_GAME_X, 0);
        lv_label_set_text(lv_obj_get_child(ttt_btns[idx], 0), "X");
        check_win();
        if(game_active) ai_move();
    }
}

static void update_score_label() {
    char buf[32];
    sprintf(buf, "Caught: %d/%d", beans_caught, MAX_BEANS);
    lv_label_set_text(lbl_score, buf);
}

static void catcher_drag_cb(lv_event_t * e) {
    lv_obj_t * obj = lv_event_get_target(e);
    lv_indev_t * indev = lv_indev_get_act();
    if(indev == NULL) return;
    lv_point_t vect;
    lv_indev_get_vect(indev, &vect);
    lv_coord_t x = lv_obj_get_x(obj) + vect.x;
    if(x < 0) x = 0;
    if(x > GAME_AREA_W - CATCHER_W) x = GAME_AREA_W - CATCHER_W;
    lv_obj_set_x(obj, x);
}

static void bean_timer_cb(lv_timer_t * t) {
    if(current_game_mode != 1) return;
    if(beans_spawned < MAX_BEANS && (rand() % 20 == 0)) {
        for(int i=0; i<MAX_BEANS; i++) {
            if(!beans[i].active && beans[i].obj == NULL) {
                beans[i].obj = lv_obj_create(bean_container);
                lv_obj_set_size(beans[i].obj, BEAN_SIZE, BEAN_SIZE);
                lv_obj_set_style_bg_color(beans[i].obj, COL_BEAN, 0);
                lv_obj_set_style_radius(beans[i].obj, LV_RADIUS_CIRCLE, 0);
                lv_obj_set_style_border_width(beans[i].obj, 0, 0);
                beans[i].x = rand() % (GAME_AREA_W - BEAN_SIZE);
                beans[i].y = -BEAN_SIZE;
                beans[i].active = true;
                lv_obj_set_pos(beans[i].obj, beans[i].x, beans[i].y);
                beans_spawned++;
                break;
            }
        }
    }
    lv_coord_t catcher_x = lv_obj_get_x(catcher_obj);
    lv_coord_t catcher_y = lv_obj_get_y(catcher_obj);
    for(int i=0; i<MAX_BEANS; i++) {
        if(beans[i].active && beans[i].obj != NULL) {
            beans[i].y += 3;
            lv_obj_set_y(beans[i].obj, beans[i].y);
            if(beans[i].y + BEAN_SIZE >= catcher_y && beans[i].y < catcher_y + CATCHER_H) {
                if(beans[i].x + BEAN_SIZE >= catcher_x && beans[i].x <= catcher_x + CATCHER_W) {
                    beans_caught++;
                    update_score_label();
                    beans[i].active = false;
                    lv_obj_add_flag(beans[i].obj, LV_OBJ_FLAG_HIDDEN);
                }
            }
            if(beans[i].y > GAME_AREA_H) {
                beans[i].active = false;
                lv_obj_add_flag(beans[i].obj, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}

static void brew_timer_cb(lv_timer_t * timer) {
    if(brew_bar == NULL) return;
    int32_t val = lv_bar_get_value(brew_bar);
    if(val < 100) {
        lv_bar_set_value(brew_bar, val + 1, LV_ANIM_ON);
        if(val == 20) lv_label_set_text(brew_label, "Grinding...");
        if(val == 50) lv_label_set_text(brew_label, "Brewing...");
        if(val == 80) lv_label_set_text(brew_label, "Adding milk...");
    } else {
        lv_label_set_text(brew_label, "COFFEE READY! ENJOY!");
        
        // Update Cloud Status to Ready
        if(current_cloud_order_id != -1) {
            update_order_status_cloud(current_cloud_order_id, "ready");
            current_cloud_order_id = -1; // Reset
        }

        lv_obj_set_style_bg_color(brew_bar, COL_SUCCESS, LV_PART_INDICATOR);
        lv_obj_clear_flag(btn_done_playing, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_opa(btn_done_playing, LV_OPA_COVER, 0);
        if(bean_timer) { lv_timer_del(bean_timer); bean_timer = NULL; }
        lv_timer_del(timer);
        brew_timer = NULL;
    }
}

// --- Style Initialization ---
void init_styles() {
    lv_style_init(&style_screen);
    lv_style_set_bg_color(&style_screen, COL_BG);
    lv_style_set_text_color(&style_screen, COL_TEXT);

    lv_style_init(&style_card);
    lv_style_set_bg_color(&style_card, COL_CARD);
    lv_style_set_radius(&style_card, 15);
    lv_style_set_border_width(&style_card, 2);
    lv_style_set_border_color(&style_card, COL_ACCENT);
    lv_style_set_pad_all(&style_card, 10);

    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, &lv_font_montserrat_24);
    lv_style_set_text_color(&style_title, COL_ACCENT);

    lv_style_init(&style_btn);
    lv_style_set_bg_color(&style_btn, COL_ACCENT);
    lv_style_set_radius(&style_btn, 10);
    lv_style_set_text_color(&style_btn, COL_BG);

    lv_style_init(&style_btn_circle);
    lv_style_set_bg_color(&style_btn_circle, lv_color_white());
    lv_style_set_radius(&style_btn_circle, LV_RADIUS_CIRCLE);
    lv_style_set_text_color(&style_btn_circle, COL_BG);

    lv_style_init(&style_btn_green);
    lv_style_set_bg_color(&style_btn_green, COL_SUCCESS);
    lv_style_set_radius(&style_btn_green, 10);
    lv_style_set_text_color(&style_btn_green, COL_BG);

    lv_style_init(&style_slider_main);
    lv_style_set_bg_color(&style_slider_main, lv_color_hex(0x333355));
    lv_style_set_radius(&style_slider_main, 10);
    lv_style_set_border_width(&style_slider_main, 1);
    lv_style_set_border_color(&style_slider_main, COL_TEXT_SEC);

    lv_style_init(&style_slider_indicator);
    lv_style_set_bg_color(&style_slider_indicator, COL_ACCENT);
    lv_style_set_bg_grad_color(&style_slider_indicator, lv_color_hex(0x0088FF));
    lv_style_set_bg_grad_dir(&style_slider_indicator, LV_GRAD_DIR_HOR);
    lv_style_set_radius(&style_slider_indicator, 10);

    lv_style_init(&style_slider_knob);
    lv_style_set_bg_color(&style_slider_knob, COL_TEXT);
    lv_style_set_bg_opa(&style_slider_knob, LV_OPA_COVER);
    lv_style_set_radius(&style_slider_knob, 50);
    lv_style_set_pad_all(&style_slider_knob, 2);

    lv_style_init(&style_game_btn);
    lv_style_set_bg_color(&style_game_btn, COL_GAME_BG);
    lv_style_set_border_width(&style_game_btn, 2);
    lv_style_set_border_color(&style_game_btn, COL_TEXT_SEC);
    lv_style_set_radius(&style_game_btn, 8);
    lv_style_set_shadow_width(&style_game_btn, 0);

    lv_style_init(&style_coupon);
    lv_style_set_bg_color(&style_coupon, COL_GOLD);
    lv_style_set_radius(&style_coupon, 8);
    lv_style_set_text_color(&style_coupon, COL_BG);
    lv_style_set_border_width(&style_coupon, 2);
    lv_style_set_border_color(&style_coupon, lv_color_white());
    lv_style_set_border_side(&style_coupon, LV_BORDER_SIDE_FULL);

    // --- Liquid Styles ---
    lv_style_init(&style_glass);
    lv_style_set_bg_color(&style_glass, lv_color_hex(0x222222));
    lv_style_set_bg_opa(&style_glass, LV_OPA_30); // Transparent
    lv_style_set_border_color(&style_glass, lv_color_white());
    lv_style_set_border_width(&style_glass, 3);
    lv_style_set_radius(&style_glass, 20); // Rounded bottom
    lv_style_set_clip_corner(&style_glass, true); // Clip liquids to glass shape

    lv_style_init(&style_liq_coffee);
    lv_style_set_bg_color(&style_liq_coffee, COL_LIQ_COFFEE_DARK);
    lv_style_set_bg_grad_color(&style_liq_coffee, COL_LIQ_COFFEE_LITE);
    lv_style_set_bg_grad_dir(&style_liq_coffee, LV_GRAD_DIR_VER);
    lv_style_set_border_width(&style_liq_coffee, 0);

    lv_style_init(&style_liq_milk);
    lv_style_set_bg_color(&style_liq_milk, COL_LIQ_MILK_DARK);
    lv_style_set_bg_grad_color(&style_liq_milk, COL_LIQ_MILK_LITE);
    lv_style_set_bg_grad_dir(&style_liq_milk, LV_GRAD_DIR_VER);
    lv_style_set_border_width(&style_liq_milk, 0);

    lv_style_init(&style_liq_foam);
    lv_style_set_bg_color(&style_liq_foam, COL_LIQ_FOAM_DARK);
    lv_style_set_bg_grad_color(&style_liq_foam, COL_LIQ_FOAM_LITE);
    lv_style_set_bg_grad_dir(&style_liq_foam, LV_GRAD_DIR_VER);
    lv_style_set_border_width(&style_liq_foam, 0);

    lv_style_init(&style_liq_choco);
    lv_style_set_bg_color(&style_liq_choco, COL_LIQ_CHOCO_DARK);
    lv_style_set_bg_grad_color(&style_liq_choco, COL_LIQ_CHOCO_LITE);
    lv_style_set_bg_grad_dir(&style_liq_choco, LV_GRAD_DIR_VER);
    lv_style_set_border_width(&style_liq_choco, 0);
}

// --- Helper Functions ---
lv_obj_t* create_back_btn(lv_obj_t* parent, lv_event_cb_t event_cb) {
    lv_obj_t * btn_back = lv_btn_create(parent);
    lv_obj_set_size(btn_back, 80, 40);
    lv_obj_align(btn_back, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_set_style_bg_color(btn_back, COL_CARD, 0);
    lv_obj_add_event_cb(btn_back, event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * lbl_back = lv_label_create(btn_back);
    lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " Back");
    lv_obj_center(lbl_back);
    return btn_back;
}

void create_slider_control(lv_obj_t* parent, const char* label_text, int y_ofs) {
    lv_obj_t * slider = lv_slider_create(parent);
    lv_obj_set_size(slider, 180, 15); 
    lv_obj_align(slider, LV_ALIGN_TOP_MID, 0, y_ofs);
    lv_obj_add_style(slider, &style_slider_main, LV_PART_MAIN);
    lv_obj_add_style(slider, &style_slider_indicator, LV_PART_INDICATOR);
    lv_obj_add_style(slider, &style_slider_knob, LV_PART_KNOB);
    lv_slider_set_value(slider, 50, LV_ANIM_OFF);

    lv_obj_t * lbl = lv_label_create(parent);
    lv_label_set_text(lbl, label_text);
    lv_obj_set_style_text_color(lbl, COL_TEXT, 0);
    lv_obj_align_to(lbl, slider, LV_ALIGN_OUT_TOP_LEFT, 0, -5);
}

// Modified to return the slider object
lv_obj_t* create_stepped_slider(lv_obj_t* parent, const char* label_text, int y_ofs, const char* steps) {
    lv_obj_t * slider = lv_slider_create(parent);
    lv_obj_set_size(slider, 180, 15); 
    lv_obj_align(slider, LV_ALIGN_TOP_MID, 0, y_ofs);
    lv_obj_add_style(slider, &style_slider_main, LV_PART_MAIN);
    lv_obj_add_style(slider, &style_slider_indicator, LV_PART_INDICATOR);
    lv_obj_add_style(slider, &style_slider_knob, LV_PART_KNOB);
    lv_slider_set_range(slider, 0, 2);
    lv_slider_set_value(slider, 1, LV_ANIM_OFF);

    lv_obj_t * lbl = lv_label_create(parent);
    lv_label_set_text(lbl, label_text);
    lv_obj_set_style_text_color(lbl, COL_TEXT, 0);
    lv_obj_align_to(lbl, slider, LV_ALIGN_OUT_TOP_LEFT, 0, -5);
    
    lv_obj_t * lbl_steps = lv_label_create(parent);
    lv_label_set_text(lbl_steps, steps);
    lv_obj_set_style_text_font(lbl_steps, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_steps, COL_TEXT_SEC, 0);
    lv_obj_align_to(lbl_steps, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    
    return slider;
}

// --- Screen: Splash ---
void create_splash_screen() {
    is_logged_in = false;
    lv_obj_t * scr = lv_obj_create(NULL);
    lv_obj_add_style(scr, &style_screen, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t * label_title = lv_label_create(scr);
    lv_label_set_text(label_title, "AuraMist Cafe");
    lv_obj_set_style_text_font(label_title, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(label_title, COL_TEXT, 0);
    lv_obj_align(label_title, LV_ALIGN_TOP_LEFT, 30, 20);

    lv_obj_t * label_sub1 = lv_label_create(scr);
    lv_label_set_text(label_sub1, "Be your own Barista !");
    lv_obj_set_style_text_font(label_sub1, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(label_sub1, COL_TEXT, 0);
    lv_obj_align(label_sub1, LV_ALIGN_TOP_LEFT, 30, 60);

    lv_obj_t * img1 = lv_img_create(scr);
    lv_img_set_src(img1, &img_Espresso);
    lv_img_set_zoom(img1, 180);
    lv_obj_align(img1, LV_ALIGN_BOTTOM_LEFT, 20, -20);

    lv_obj_t * img2 = lv_img_create(scr);
    lv_img_set_src(img2, &img_Latte);
    lv_img_set_zoom(img2, 140);
    lv_obj_align(img2, LV_ALIGN_BOTTOM_LEFT, 100, -20);

    lv_obj_t * img3 = lv_img_create(scr);
    lv_img_set_src(img3, &img_Americano);
    lv_img_set_zoom(img3, 180);
    lv_obj_align(img3, LV_ALIGN_BOTTOM_LEFT, 180, -20);

    lv_obj_t * btn = lv_btn_create(scr);
    lv_obj_set_size(btn, 80, 80);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -40, 0);
    lv_obj_add_style(btn, &style_btn_circle, 0);
    lv_obj_add_event_cb(btn, btn_start_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * icon = lv_label_create(btn);
    lv_label_set_text(icon, LV_SYMBOL_PLAY);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_32, 0);
    lv_obj_center(icon);

    lv_obj_t * label_btn = lv_label_create(scr);
    lv_label_set_text(label_btn, "Get started");
    lv_obj_set_style_text_font(label_btn, &lv_font_montserrat_20, 0);
    lv_obj_align_to(label_btn, btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    // --- Pickup Button ---
    lv_obj_t * btn_pickup = lv_btn_create(scr);
    lv_obj_set_size(btn_pickup, 140, 45);
    lv_obj_align(btn_pickup, LV_ALIGN_TOP_RIGHT, -20, 20);
    lv_obj_add_style(btn_pickup, &style_btn, 0);
    lv_obj_set_style_bg_color(btn_pickup, COL_CARD, 0); 
    lv_obj_set_style_border_width(btn_pickup, 2, 0);
    lv_obj_set_style_border_color(btn_pickup, COL_ACCENT, 0);
    lv_obj_add_event_cb(btn_pickup, btn_pickup_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * lbl_pickup = lv_label_create(btn_pickup);
    lv_label_set_text(lbl_pickup, "Pickup Order");
    lv_obj_set_style_text_color(lbl_pickup, COL_ACCENT, 0);
    lv_obj_center(lbl_pickup);

    switch_to_screen(scr);
}

// --- Screen: Menu ---
void create_menu_screen() {
    lv_obj_t * scr = lv_obj_create(NULL);
    lv_obj_add_style(scr, &style_screen, 0);
    create_back_btn(scr, btn_home_cb);

    lv_obj_t * header = lv_label_create(scr);
    lv_label_set_text(header, "Select your drink");
    lv_obj_add_style(header, &style_title, 0);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 15);

    lv_obj_t * cont = lv_obj_create(scr);
    lv_obj_set_size(cont, 480, 260);
    lv_obj_align(cont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_gap(cont, 20, 0);
    lv_obj_set_style_pad_hor(cont, 135, 0); 
    lv_obj_set_scroll_snap_x(cont, LV_SCROLL_SNAP_CENTER);
    
    // Removed SCROLL_ONE flag for smoother/freer scrolling
    // lv_obj_add_flag(cont, LV_OBJ_FLAG_SCROLL_ONE); 
    lv_obj_set_scroll_dir(cont, LV_DIR_HOR);

    for(int i = 0; i < 4; i++) {
        lv_obj_t * card = lv_btn_create(cont);
        lv_obj_set_size(card, 210, 220);
        lv_obj_set_style_bg_opa(card, LV_OPA_0, 0);
        lv_obj_set_style_border_width(card, 0, 0);
        lv_obj_add_event_cb(card, btn_coffee_select_cb, LV_EVENT_CLICKED, (void*)(intptr_t)i);
        lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t * img = lv_img_create(card);
        lv_img_set_src(img, coffee_images[i]);
        lv_obj_align(img, LV_ALIGN_LEFT_MID, 0, -20);
        lv_img_set_zoom(img, coffee_zooms[i]); 

        lv_obj_t * lbl_price = lv_label_create(card);
        lv_label_set_text(lbl_price, coffee_prices[i]);
        lv_obj_set_style_text_font(lbl_price, &lv_font_montserrat_24, 0);
        lv_obj_align(lbl_price, LV_ALIGN_RIGHT_MID, -10, -20);

        lv_obj_t * line_top = lv_obj_create(card);
        lv_obj_set_size(line_top, 70, 2);
        lv_obj_set_style_bg_color(line_top, COL_TEXT, 0);
        lv_obj_set_style_border_width(line_top, 0, 0);
        lv_obj_align_to(line_top, lbl_price, LV_ALIGN_OUT_TOP_MID, 0, -5);

        lv_obj_t * line_bot = lv_obj_create(card);
        lv_obj_set_size(line_bot, 70, 2);
        lv_obj_set_style_bg_color(line_bot, COL_TEXT, 0);
        lv_obj_set_style_border_width(line_bot, 0, 0);
        lv_obj_align_to(line_bot, lbl_price, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

        lv_obj_t * lbl_name = lv_label_create(card);
        lv_label_set_text(lbl_name, coffee_names[i]);
        lv_obj_set_style_text_font(lbl_name, &lv_font_montserrat_20, 0);
        lv_obj_align(lbl_name, LV_ALIGN_BOTTOM_MID, 0, 0);
    }
    switch_to_screen(scr);
}

// --- Screen: Make it Yours (New!) ---
void create_make_it_yours_screen() {
    lv_obj_t * scr = lv_obj_create(NULL);
    lv_obj_add_style(scr, &style_screen, 0);
    create_back_btn(scr, btn_back_to_menu_cb);

    // --- Left Side: The Living Cup ---
    glass_obj = lv_obj_create(scr);
    lv_obj_set_size(glass_obj, 140, 240);
    lv_obj_align(glass_obj, LV_ALIGN_LEFT_MID, 30, 40); // Moved down
    lv_obj_add_style(glass_obj, &style_glass, 0);
    
    // Use COLUMN flow (Top to Bottom) but align to END (Bottom)
    // This stacks items at the bottom of the glass.
    lv_obj_set_flex_flow(glass_obj, LV_FLEX_FLOW_COLUMN); 
    lv_obj_set_flex_align(glass_obj, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_set_style_pad_all(glass_obj, 0, 0);
    lv_obj_set_style_pad_gap(glass_obj, 0, 0);
    lv_obj_clear_flag(glass_obj, LV_OBJ_FLAG_SCROLLABLE); // Disable scroll

    // Liquid Layers (Initialized to 0 height)
    // Order: Foam (Top) -> Milk -> Choco -> Coffee (Bottom)
    // Created in this order so they stack correctly in COLUMN flow.
    
    liq_foam = lv_obj_create(glass_obj);
    lv_obj_set_width(liq_foam, 140);
    lv_obj_set_height(liq_foam, 0);
    lv_obj_add_style(liq_foam, &style_liq_foam, 0);
    lv_obj_add_flag(liq_foam, LV_OBJ_FLAG_CLICKABLE);
    // Event added later

    liq_milk = lv_obj_create(glass_obj);
    lv_obj_set_width(liq_milk, 140);
    lv_obj_set_height(liq_milk, 0);
    lv_obj_add_style(liq_milk, &style_liq_milk, 0);
    lv_obj_add_flag(liq_milk, LV_OBJ_FLAG_CLICKABLE);
    // Event added later

    liq_choco = lv_obj_create(glass_obj);
    lv_obj_set_width(liq_choco, 140);
    lv_obj_set_height(liq_choco, 0);
    lv_obj_add_style(liq_choco, &style_liq_choco, 0);
    lv_obj_add_flag(liq_choco, LV_OBJ_FLAG_CLICKABLE);
    // Event added later

    liq_coffee = lv_obj_create(glass_obj);
    lv_obj_set_width(liq_coffee, 140);
    lv_obj_set_height(liq_coffee, 0);
    lv_obj_add_style(liq_coffee, &style_liq_coffee, 0);
    lv_obj_add_flag(liq_coffee, LV_OBJ_FLAG_CLICKABLE);
    // Event added later to pass source button

    // --- Right Side: Controls ---
    right_panel_miy = lv_obj_create(scr);
    lv_obj_set_size(right_panel_miy, 260, 320); // Increased height
    lv_obj_align(right_panel_miy, LV_ALIGN_RIGHT_MID, -10, -10);
    lv_obj_set_style_bg_opa(right_panel_miy, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(right_panel_miy, 0, 0);

    // Emotional Title - Reparented to screen to float over glass
    lbl_mood = lv_label_create(scr); 
    lv_label_set_text(lbl_mood, "");
    lv_obj_set_style_text_font(lbl_mood, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(lbl_mood, COL_GOLD, 0); // Gold color for "pop"
    lv_obj_set_style_bg_color(lbl_mood, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(lbl_mood, LV_OPA_60, 0); // Semi-transparent background
    lv_obj_set_style_pad_all(lbl_mood, 5, 0);
    lv_obj_set_style_radius(lbl_mood, 5, 0);
    
    // Align to top center of screen (same row as back button)
    lv_obj_align(lbl_mood, LV_ALIGN_TOP_MID, 0, 15);
    
    lv_obj_add_flag(lbl_mood, LV_OBJ_FLAG_HIDDEN); // Initially hidden
    lv_obj_clear_flag(lbl_mood, LV_OBJ_FLAG_CLICKABLE); // Don't block clicks
    lv_obj_add_event_cb(lbl_mood, lbl_mood_delete_cb, LV_EVENT_DELETE, NULL);

    // Draggable Ingredients Grid
    int grid_w = 100;
    int grid_h = 60;
    int gap_x = 20;
    int gap_y = 15; // Reduced gap to make space for title
    int start_y = 10; // Moved up to make space at bottom

    // Row 1: Coffee | Milk
    // Coffee Container (Draggable Group)
    lv_obj_t * cnt_coffee = lv_obj_create(right_panel_miy);
    lv_obj_set_size(cnt_coffee, grid_w, grid_h + 35);
    lv_obj_set_pos(cnt_coffee, 0, start_y);
    lv_obj_set_style_bg_opa(cnt_coffee, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cnt_coffee, 0, 0);
    lv_obj_set_style_pad_all(cnt_coffee, 0, 0);
    lv_obj_add_flag(cnt_coffee, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK); // Make container draggable

    lv_obj_t * btn_d_coffee = lv_btn_create(cnt_coffee);
    lv_obj_set_size(btn_d_coffee, grid_w, grid_h);
    lv_obj_set_style_bg_color(btn_d_coffee, COL_LIQ_COFFEE_LITE, 0);
    lv_obj_clear_flag(btn_d_coffee, LV_OBJ_FLAG_CLICKABLE); // Pass clicks to container
    lv_obj_t * l1 = lv_label_create(btn_d_coffee); lv_label_set_text(l1, "Coffee"); lv_obj_center(l1); lv_obj_set_style_text_color(l1, COL_TEXT, 0);
    
    // Coffee Dropdown
    dd_coffee_type = lv_dropdown_create(cnt_coffee);
    lv_dropdown_set_options(dd_coffee_type, "Hot Light\nHot Medium\nHot Dark\nWarm Light\nWarm Medium\nWarm Dark\nCold Light\nCold Medium\nCold Dark");
    lv_obj_set_width(dd_coffee_type, grid_w);
    lv_obj_align(dd_coffee_type, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_text_font(dd_coffee_type, &lv_font_montserrat_10, 0);

    // Setup Drag for Coffee Container
    dc_coffee.origin_x = 0; dc_coffee.origin_y = start_y; dc_coffee.target_liq_ptr = &liq_coffee; dc_coffee.dropdown_ref = dd_coffee_type;
    lv_obj_add_event_cb(cnt_coffee, source_drag_event_cb, LV_EVENT_ALL, &dc_coffee);

    // Milk Container (Draggable Group)
    lv_obj_t * cnt_milk = lv_obj_create(right_panel_miy);
    lv_obj_set_size(cnt_milk, grid_w, grid_h + 35);
    lv_obj_set_pos(cnt_milk, grid_w + gap_x, start_y);
    lv_obj_set_style_bg_opa(cnt_milk, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cnt_milk, 0, 0);
    lv_obj_set_style_pad_all(cnt_milk, 0, 0);
    lv_obj_add_flag(cnt_milk, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK);

    lv_obj_t * btn_d_milk = lv_btn_create(cnt_milk);
    lv_obj_set_size(btn_d_milk, grid_w, grid_h);
    lv_obj_set_style_bg_color(btn_d_milk, COL_LIQ_MILK_LITE, 0);
    lv_obj_set_style_text_color(btn_d_milk, COL_BG, 0);
    lv_obj_clear_flag(btn_d_milk, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t * l2 = lv_label_create(btn_d_milk); lv_label_set_text(l2, "Milk"); lv_obj_center(l2); lv_obj_set_style_text_color(l2, COL_BG, 0);

    // Milk Dropdown
    dd_milk_type = lv_dropdown_create(cnt_milk);
    lv_dropdown_set_options(dd_milk_type, "Whole\nOat\nAlmond");
    lv_obj_set_width(dd_milk_type, grid_w);
    lv_obj_align(dd_milk_type, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_text_font(dd_milk_type, &lv_font_montserrat_10, 0);

    // Setup Drag for Milk Container
    dc_milk.origin_x = grid_w + gap_x; dc_milk.origin_y = start_y; dc_milk.target_liq_ptr = &liq_milk; dc_milk.dropdown_ref = dd_milk_type;
    lv_obj_add_event_cb(cnt_milk, source_drag_event_cb, LV_EVENT_ALL, &dc_milk);

    // Row 2: Foam | Choco
    start_y += grid_h + gap_y + 35; // Adjust for dropdowns
    
    // Foam
    lv_obj_t * btn_d_foam = lv_btn_create(right_panel_miy);
    lv_obj_set_size(btn_d_foam, grid_w, grid_h);
    lv_obj_set_pos(btn_d_foam, 0, start_y);
    lv_obj_set_style_bg_color(btn_d_foam, COL_LIQ_FOAM_LITE, 0);
    lv_obj_set_style_text_color(btn_d_foam, COL_BG, 0);
    lv_obj_add_flag(btn_d_foam, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK);
    dc_foam.origin_x = 0; dc_foam.origin_y = start_y; dc_foam.target_liq_ptr = &liq_foam; dc_foam.dropdown_ref = NULL;
    lv_obj_add_event_cb(btn_d_foam, source_drag_event_cb, LV_EVENT_ALL, &dc_foam);
    lv_obj_t * l3 = lv_label_create(btn_d_foam); lv_label_set_text(l3, "Foam"); lv_obj_center(l3); lv_obj_set_style_text_color(l3, COL_BG, 0);

    // Choco
    lv_obj_t * btn_d_choco = lv_btn_create(right_panel_miy);
    lv_obj_set_size(btn_d_choco, grid_w, grid_h);
    lv_obj_set_pos(btn_d_choco, grid_w + gap_x, start_y);
    lv_obj_set_style_bg_color(btn_d_choco, COL_LIQ_CHOCO_LITE, 0);
    lv_obj_add_flag(btn_d_choco, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_PRESS_LOCK);
    dc_choco.origin_x = grid_w + gap_x; dc_choco.origin_y = start_y; dc_choco.target_liq_ptr = &liq_choco; dc_choco.dropdown_ref = NULL;
    lv_obj_add_event_cb(btn_d_choco, source_drag_event_cb, LV_EVENT_ALL, &dc_choco);
    lv_obj_t * l4 = lv_label_create(btn_d_choco); lv_label_set_text(l4, "Choco"); lv_obj_center(l4); lv_obj_set_style_text_color(l4, COL_TEXT, 0);

    // Attach Liquid Events with Source Container Context
    lv_obj_add_event_cb(liq_coffee, layer_drag_event_cb, LV_EVENT_ALL, cnt_coffee);
    lv_obj_add_event_cb(liq_milk, layer_drag_event_cb, LV_EVENT_ALL, cnt_milk);
    lv_obj_add_event_cb(liq_foam, layer_drag_event_cb, LV_EVENT_ALL, btn_d_foam);
    lv_obj_add_event_cb(liq_choco, layer_drag_event_cb, LV_EVENT_ALL, btn_d_choco);

    // Naming & Saving
    ta_recipe_name = lv_textarea_create(right_panel_miy);
    lv_textarea_set_one_line(ta_recipe_name, true);
    lv_textarea_set_placeholder_text(ta_recipe_name, "Name your brew...");
    lv_obj_set_size(ta_recipe_name, 180, 40);
    lv_obj_align(ta_recipe_name, LV_ALIGN_BOTTOM_LEFT, 0, -60); // Moved down
    lv_obj_add_event_cb(ta_recipe_name, ta_recipe_focus_cb, LV_EVENT_ALL, NULL); // Add focus listener

    btn_save_recipe = lv_btn_create(right_panel_miy);
    lv_obj_set_size(btn_save_recipe, 60, 40);
    lv_obj_align(btn_save_recipe, LV_ALIGN_BOTTOM_RIGHT, 0, -60); // Moved down
    lv_obj_add_style(btn_save_recipe, &style_btn_green, 0);
    lv_obj_add_event_cb(btn_save_recipe, btn_save_recipe_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * l_save = lv_label_create(btn_save_recipe);
    lv_label_set_text(l_save, "Save");
    lv_obj_center(l_save);

    // Checkout Button
    lv_obj_t * btn_pay = lv_btn_create(right_panel_miy);
    lv_obj_set_size(btn_pay, 140, 40);
    lv_obj_align(btn_pay, LV_ALIGN_BOTTOM_MID, 0, -5); // Moved down
    lv_obj_add_style(btn_pay, &style_btn, 0);
    lv_obj_add_event_cb(btn_pay, btn_checkout_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * l_pay = lv_label_create(btn_pay);
    lv_label_set_text(l_pay, "Brew This!");
    lv_obj_center(l_pay);

    // Keyboard (Hidden)
    kb_recipe = lv_keyboard_create(scr);
    lv_keyboard_set_textarea(kb_recipe, ta_recipe_name);
    lv_obj_add_flag(kb_recipe, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(kb_recipe, kb_recipe_cb, LV_EVENT_READY, NULL);
    lv_obj_add_event_cb(kb_recipe, kb_recipe_cb, LV_EVENT_CANCEL, NULL);

    update_mood_label(); // Init visuals
    switch_to_screen(scr);
}

// --- Screen: Customize (Standard) ---
void create_customize_screen() {
    lv_obj_t * scr = lv_obj_create(NULL);
    lv_obj_add_style(scr, &style_screen, 0);
    create_back_btn(scr, btn_back_to_menu_cb);

    lv_obj_t * img_ill = lv_img_create(scr);
    lv_img_set_src(img_ill, coffee_images[selected_coffee_index]);
    lv_obj_align(img_ill, LV_ALIGN_LEFT_MID, 20, 0); 
    lv_img_set_zoom(img_ill, coffee_zooms[selected_coffee_index]); 

    lv_obj_t * right_panel = lv_obj_create(scr);
    lv_obj_set_size(right_panel, 240, 310); 
    lv_obj_align(right_panel, LV_ALIGN_RIGHT_MID, -5, 0);
    lv_obj_set_style_bg_opa(right_panel, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(right_panel, 0, 0);

    lv_obj_t * header = lv_label_create(right_panel);
    lv_label_set_text(header, coffee_names[selected_coffee_index]);
    lv_obj_add_style(header, &style_title, 0);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);

    int y_start = 50;
    int y_step = 55;

    // Unified Logic for Espresso (1), Americano (2), Latte (3)
    if(selected_coffee_index > 0) { 
        create_stepped_slider(right_panel, "Strength", y_start, "Light      Medium      Dark");
        create_stepped_slider(right_panel, "Temperature", y_start + y_step, "Cold       Warm       Hot");
        
        // Added Size Slider
        slider_size = create_stepped_slider(right_panel, "Size", y_start + y_step*2, "Small   Medium(+$0.5)   Large(+$1)");
    } else {
        // Fallback for other types if added later
        create_slider_control(right_panel, "Strength", y_start);
        create_slider_control(right_panel, "Temperature", y_start + y_step);
    }

    lv_obj_t * btn_pay = lv_btn_create(right_panel);
    lv_obj_set_size(btn_pay, 140, 50);
    lv_obj_align(btn_pay, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_add_style(btn_pay, &style_btn, 0);
    lv_obj_add_event_cb(btn_pay, btn_checkout_cb, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t * l2 = lv_label_create(btn_pay);
    lv_label_set_text(l2, "Checkout");
    lv_obj_center(l2);
    switch_to_screen(scr);
}

// --- Payment Logic ---
void create_payment_screen() {
    lv_obj_t * scr = lv_obj_create(NULL);
    lv_obj_add_style(scr, &style_screen, 0);
    create_back_btn(scr, btn_back_to_customize_cb);

    panel_payment = lv_obj_create(scr);
    lv_obj_set_size(panel_payment, 320, 240);
    lv_obj_align(panel_payment, LV_ALIGN_CENTER, 0, 20); 
    lv_obj_add_style(panel_payment, &style_card, 0);

    lv_obj_t * title = lv_label_create(panel_payment);
    lv_label_set_text(title, "Payment");
    lv_obj_add_style(title, &style_title, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 0);

    lbl_total_pay = lv_label_create(panel_payment);
    update_price_label();
    lv_obj_set_style_text_font(lbl_total_pay, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(lbl_total_pay, COL_TEXT, 0); 
    lv_obj_align(lbl_total_pay, LV_ALIGN_CENTER, 0, -30);

    lv_obj_t * btn_pay = lv_btn_create(panel_payment);
    lv_obj_set_size(btn_pay, 200, 50);
    lv_obj_align(btn_pay, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_add_style(btn_pay, &style_btn_green, 0);
    lv_obj_add_event_cb(btn_pay, btn_pay_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * lbl = lv_label_create(btn_pay);
    lv_label_set_text(lbl, "Pay Now");
    lv_obj_center(lbl);

    btn_login_main = lv_btn_create(panel_payment);
    lv_obj_set_size(btn_login_main, 200, 40);
    lv_obj_align(btn_login_main, LV_ALIGN_BOTTOM_MID, 0, -70);
    lv_obj_set_style_bg_color(btn_login_main, is_logged_in ? COL_SUCCESS : COL_WARNING, 0);
    lv_obj_add_event_cb(btn_login_main, btn_login_toggle_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * lbl_log = lv_label_create(btn_login_main);
    lv_label_set_text(lbl_log, is_logged_in ? "Enter Promo Code" : "Member Login (Deals)");
    lv_obj_center(lbl_log);

    ta_login = lv_textarea_create(scr);
    lv_textarea_set_one_line(ta_login, true);
    lv_textarea_set_placeholder_text(ta_login, "Enter Phone Number");
    lv_obj_set_size(ta_login, 250, 40);
    lv_obj_align(ta_login, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_add_flag(ta_login, LV_OBJ_FLAG_HIDDEN);

    ta_promo = lv_textarea_create(scr);
    lv_textarea_set_one_line(ta_promo, true);
    lv_textarea_set_placeholder_text(ta_promo, "Enter Promo Code");
    lv_obj_set_size(ta_promo, 250, 40);
    lv_obj_align(ta_promo, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_add_flag(ta_promo, LV_OBJ_FLAG_HIDDEN);

    kb_input = lv_keyboard_create(scr);
    lv_obj_add_flag(kb_input, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(kb_input, kb_event_cb, LV_EVENT_READY, NULL);
    lv_obj_add_event_cb(kb_input, kb_event_cb, LV_EVENT_CANCEL, NULL);
    
    switch_to_screen(scr);
}

// --- Screen: Game (Tic Tac Toe + Beans) ---
void create_game_screen() {
    lv_obj_t * scr = lv_obj_create(NULL);
    lv_obj_add_style(scr, &style_screen, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE); 

    // Reset Games
    for(int i=0; i<9; i++) ttt_board[i] = 0;
    game_active = true;
    beans_caught = 0;
    beans_spawned = 0;
    current_game_mode = 0;
    ttt_discount = 0;
    for(int i=0; i<MAX_BEANS; i++) { beans[i].active = false; beans[i].obj = NULL; }

    lv_obj_t * game_title_label = lv_label_create(scr);
    lv_label_set_text(game_title_label, "Play Tic-Tac-Toe!");
    lv_obj_add_style(game_title_label, &style_title, 0);
    lv_obj_align(game_title_label, LV_ALIGN_TOP_MID, 0, 5);

    // --- Tic Tac Toe Container ---
    ttt_container = lv_obj_create(scr);
    lv_obj_set_size(ttt_container, GAME_AREA_W, GAME_AREA_H);
    lv_obj_align(ttt_container, LV_ALIGN_TOP_MID, 0, 45); 
    lv_obj_set_style_bg_color(ttt_container, COL_CARD, 0);
    lv_obj_set_style_border_color(ttt_container, COL_TEXT_SEC, 0);
    lv_obj_set_style_border_width(ttt_container, 2, 0);
    lv_obj_clear_flag(ttt_container, LV_OBJ_FLAG_SCROLLABLE);

    int start_x = 57;
    int start_y = 17;
    int btn_size = 50;
    int gap = 8;

    for(int i=0; i<9; i++) {
        ttt_btns[i] = lv_btn_create(ttt_container);
        lv_obj_set_size(ttt_btns[i], btn_size, btn_size);
        lv_obj_set_pos(ttt_btns[i], start_x + (i%3)*(btn_size+gap), start_y + (i/3)*(btn_size+gap)); 
        lv_obj_add_style(ttt_btns[i], &style_game_btn, 0);
        lv_obj_add_event_cb(ttt_btns[i], ttt_btn_cb, LV_EVENT_CLICKED, (void*)(intptr_t)i);
        lv_obj_t * lbl = lv_label_create(ttt_btns[i]);
        lv_label_set_text(lbl, "");
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_32, 0); 
        lv_obj_center(lbl);
    }

    // --- Catch Beans Container ---
    bean_container = lv_obj_create(scr);
    lv_obj_set_size(bean_container, GAME_AREA_W, GAME_AREA_H);
    lv_obj_align(bean_container, LV_ALIGN_TOP_MID, 0, 45); 
    lv_obj_set_style_bg_color(bean_container, COL_CARD, 0);
    lv_obj_set_style_border_color(bean_container, COL_TEXT_SEC, 0);
    lv_obj_set_style_border_width(bean_container, 2, 0);
    lv_obj_clear_flag(bean_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(bean_container, LV_OBJ_FLAG_HIDDEN); 

    lbl_score = lv_label_create(bean_container);
    lv_label_set_text(lbl_score, "Caught: 0/10");
    lv_obj_set_style_text_color(lbl_score, COL_TEXT, 0); 
    lv_obj_align(lbl_score, LV_ALIGN_TOP_RIGHT, -10, 5);

    catcher_obj = lv_obj_create(bean_container);
    lv_obj_set_size(catcher_obj, CATCHER_W, CATCHER_H);
    lv_obj_set_pos(catcher_obj, (GAME_AREA_W - CATCHER_W)/2, GAME_AREA_H - CATCHER_H - 10);
    lv_obj_set_style_bg_color(catcher_obj, COL_CATCHER, 0);
    lv_obj_clear_flag(catcher_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(catcher_obj, catcher_drag_cb, LV_EVENT_PRESSING, NULL);

    // Brewing Status
    brew_label = lv_label_create(scr);
    lv_label_set_text(brew_label, "Heating water...");
    lv_obj_align(brew_label, LV_ALIGN_BOTTOM_MID, 0, -50);

    brew_bar = lv_bar_create(scr);
    lv_obj_set_size(brew_bar, 300, 20); 
    lv_obj_align(brew_bar, LV_ALIGN_BOTTOM_MID, 0, -25);
    lv_obj_add_style(brew_bar, &style_slider_main, LV_PART_MAIN);
    lv_obj_add_style(brew_bar, &style_slider_indicator, LV_PART_INDICATOR);
    lv_bar_set_value(brew_bar, 0, LV_ANIM_OFF);

    btn_done_playing = lv_btn_create(scr);
    lv_obj_set_size(btn_done_playing, 60, 60);
    lv_obj_align(btn_done_playing, LV_ALIGN_RIGHT_MID, -10, 0); 
    lv_obj_add_style(btn_done_playing, &style_btn_circle, 0);
    lv_obj_set_style_bg_color(btn_done_playing, COL_SUCCESS, 0);
    lv_obj_add_event_cb(btn_done_playing, btn_done_playing_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(btn_done_playing, LV_OBJ_FLAG_HIDDEN);
    
    lv_obj_t * l = lv_label_create(btn_done_playing);
    lv_label_set_text(l, LV_SYMBOL_RIGHT);
    lv_obj_set_style_text_font(l, &lv_font_montserrat_32, 0);
    lv_obj_center(l);

    brew_timer = lv_timer_create(brew_timer_cb, 300, NULL);
    bean_timer = lv_timer_create(bean_timer_cb, 50, NULL); 

    switch_to_screen(scr);
}

// --- Screen: Enjoy ---
void create_enjoy_screen() {
    lv_obj_t * scr = lv_obj_create(NULL);
    lv_obj_add_style(scr, &style_screen, 0);

    lv_obj_t * title = lv_label_create(scr);
    lv_label_set_text(title, "Enjoy your Coffee!");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(title, COL_SUCCESS, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    lv_obj_t * msg = lv_label_create(scr);
    lv_label_set_text(msg, "May your coffee be strong\nand your Monday be short!");
    lv_obj_set_style_text_align(msg, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(msg, &lv_font_montserrat_20, 0);
    lv_obj_align(msg, LV_ALIGN_TOP_MID, 0, 70);

    int bean_discount = beans_caught; 
    int final_discount = 0;
    const char* coupon_code = "";
    
    if (bean_discount > ttt_discount) {
        final_discount = bean_discount;
        static char b_code[16];
        sprintf(b_code, "BEAN%d", final_discount);
        coupon_code = b_code;
    } else {
        final_discount = ttt_discount;
        if(final_discount == 10) coupon_code = "TIC10";
        else coupon_code = "TIC5";
    }

    lv_obj_t * coupon_box = lv_obj_create(scr);
    lv_obj_set_size(coupon_box, 300, 100);
    lv_obj_align(coupon_box, LV_ALIGN_CENTER, 0, 20);
    lv_obj_add_style(coupon_box, &style_coupon, 0);
    
    lv_obj_t * lbl_disc = lv_label_create(coupon_box);
    char buf[64];
    sprintf(buf, "YOU WON %d%% OFF!", final_discount);
    lv_label_set_text(lbl_disc, buf);
    lv_obj_set_style_text_font(lbl_disc, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(lbl_disc, COL_BG, 0);
    lv_obj_align(lbl_disc, LV_ALIGN_TOP_MID, 0, 5);
    
    lv_obj_t * lbl_code = lv_label_create(coupon_box);
    sprintf(buf, "CODE: %s", coupon_code);
    lv_label_set_text(lbl_code, buf);
    lv_obj_set_style_text_font(lbl_code, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(lbl_code, COL_BG, 0);
    lv_obj_align(lbl_code, LV_ALIGN_BOTTOM_MID, 0, -5);

    lv_obj_t * btn = lv_btn_create(scr);
    lv_obj_set_size(btn, 160, 50);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_obj_add_style(btn, &style_btn, 0);
    lv_obj_add_event_cb(btn, btn_home_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * l = lv_label_create(btn);
    lv_label_set_text(l, "Home");
    lv_obj_center(l);

    switch_to_screen(scr);
}

// --- New: Pickup Screen Logic ---
static lv_obj_t * ta_pickup_code = NULL;
static lv_obj_t * kb_pickup = NULL;
static lv_obj_t * lbl_pickup_status = NULL;

static void btn_pickup_confirm_cb(lv_event_t * e) {
    const char* code_str = lv_textarea_get_text(ta_pickup_code);
    if(strlen(code_str) < 4) {
        lv_label_set_text(lbl_pickup_status, "Enter 4-digit code");
        return;
    }
    
    int orderId = atoi(code_str);
    lv_label_set_text(lbl_pickup_status, "Fetching Order...");
    lv_task_handler(); // Force UI update
    
    // Call External Function
    CloudOrder order = fetch_order_from_cloud(orderId);
    
    if(order.valid) {
        lv_label_set_text(lbl_pickup_status, "Order Found!");
        
        // Populate Global State
        selected_coffee_index = 0; // Treat as Custom
        final_price = order.price;
        current_cloud_order_id = orderId; // Store ID for later updates
        
        // We need to set the liquid levels for the visual cup
        // Since we jump to Payment, we might not show the cup, but let's set them anyway
        // Or better, we jump to Payment Screen directly as requested.
        
        create_payment_screen();
    } else {
        lv_label_set_text(lbl_pickup_status, "Order Not Found!");
    }
}

static void kb_pickup_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_READY) {
        btn_pickup_confirm_cb(NULL);
    } else if(code == LV_EVENT_CANCEL) {
        create_splash_screen();
    }
}

void create_pickup_screen() {
    lv_obj_t * scr = lv_obj_create(NULL);
    lv_obj_add_style(scr, &style_screen, 0);
    create_back_btn(scr, btn_home_cb);
    
    lv_obj_t * title = lv_label_create(scr);
    lv_label_set_text(title, "Enter Order ID");
    lv_obj_add_style(title, &style_title, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    
    ta_pickup_code = lv_textarea_create(scr);
    lv_textarea_set_one_line(ta_pickup_code, true);
    lv_textarea_set_max_length(ta_pickup_code, 4);
    lv_textarea_set_placeholder_text(ta_pickup_code, "0000");
    lv_obj_set_size(ta_pickup_code, 140, 50);
    lv_obj_align(ta_pickup_code, LV_ALIGN_TOP_MID, -40, 60);
    lv_obj_set_style_text_font(ta_pickup_code, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_align(ta_pickup_code, LV_TEXT_ALIGN_CENTER, 0);
    
    // Manual Confirm Button
    lv_obj_t * btn_go = lv_btn_create(scr);
    lv_obj_set_size(btn_go, 70, 50);
    lv_obj_align(btn_go, LV_ALIGN_TOP_MID, 75, 60);
    lv_obj_add_style(btn_go, &style_btn_green, 0);
    lv_obj_add_event_cb(btn_go, btn_pickup_confirm_cb, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t * l_go = lv_label_create(btn_go);
    lv_label_set_text(l_go, "GO");
    lv_obj_center(l_go);

    lbl_pickup_status = lv_label_create(scr);
    lv_label_set_text(lbl_pickup_status, "");
    lv_obj_set_style_text_color(lbl_pickup_status, COL_WARNING, 0);
    lv_obj_align(lbl_pickup_status, LV_ALIGN_TOP_MID, 0, 120);
    
    kb_pickup = lv_keyboard_create(scr);
    lv_keyboard_set_mode(kb_pickup, LV_KEYBOARD_MODE_NUMBER);
    lv_keyboard_set_textarea(kb_pickup, ta_pickup_code);
    lv_obj_add_event_cb(kb_pickup, kb_pickup_cb, LV_EVENT_ALL, NULL);
    
    switch_to_screen(scr);
}

void coffee_app_init() {
    init_styles();
    create_splash_screen();
    // Attempt to clean system layer to remove FPS monitor if possible (best effort)
    // Note: Usually requires lv_conf.h change to disable LV_USE_PERF_MONITOR
    lv_obj_clean(lv_layer_sys()); 
}