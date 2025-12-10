#ifndef COFFEE_APP_H
#define COFFEE_APP_H

#include <lvgl.h>

// --- Data Structures ---
struct CustomRecipe {
    char name[32];
    int coffee_val;
    int milk_val;
    int foam_val;
};

// --- Cloud Order Structure ---
struct CloudOrder {
    bool valid;
    int id;
    char item_name[32];
    float price;
    // Ingredients levels (0-100 scale based on flex value)
    // 0:espresso, 1:milk_whole, 2:milk_almond, 3:milk_oat, 4:foam, 5:water, 6:syrup, 7:ice
    float ingredients[8]; 
};

// --- Public Functions ---
void coffee_app_init();

// --- External Network Functions (Implemented in .ino) ---
extern bool is_wifi_connected();
extern CloudOrder fetch_order_from_cloud(int orderId);
extern void update_order_status_cloud(int orderId, const char* status);

#endif // COFFEE_APP_H