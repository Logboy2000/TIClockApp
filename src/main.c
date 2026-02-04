#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include <sys/rtc.h>
#include <graphx.h>
#include <keypadc.h>
#include <sys/power.h>

// Digit Sprites
#include "gfx/gfx.h"

#define SCREEN_W 320
#define SCREEN_H 240
#define SCREEN_W_HALF 160
#define SCREEN_H_HALF 120
#define TEXT_HEIGHT 8

void draw(void);

const char* days_of_week[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const char* months_of_year[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const char* date_postfixes[] = {"th", "st", "nd", "rd"};

uint8_t hrs = 0;
uint8_t mins = 0;
uint8_t secs = 0;
uint8_t last_secs = 0;

float clock_sx = 3;
float clock_sy = 3;
float analog_clock_scale = 1.75;

// 4 toggle options with F1-F4 keys
bool ampm_clock = true;
bool show_secs = true;
bool show_date = true;
bool analog_clock = true;
bool show_help = false;

bool seen_help_message = false;

char notification_msg[50];
int notification_timer = 0;

static uint8_t prev_kb_Data[8];
bool kb_JustPressed(kb_lkey_t key)
{
    uint8_t key_group = (key >> 8) & 7;
    uint8_t key_mask = key & 0xFF;

    bool was_down = prev_kb_Data[key_group] & key_mask;
    bool is_down = kb_IsDown(key);

    return !was_down && is_down;
}

bool step(void)
{
    bool do_draw = false;

    if (notification_timer > 0) {
        notification_timer--;
        if (notification_timer == 0) do_draw = true;
    }
    if (!seen_help_message) {
        seen_help_message = true;
        sprintf(notification_msg, "Press [F5] for help");
        notification_timer = 5000;
        do_draw = true;
    }


    // Save previous keyboard state
    for (int i = 1; i <= 7; i++)
    {
        prev_kb_Data[i] = kb_Data[i];
    }

    // Scan keyboard
    kb_Scan();

    // Quit on CLEAR
    if (kb_IsDown(kb_KeyClear))
    {
        return false;
    }
    if (kb_IsDown(kb_KeyAdd))
    {
        if (analog_clock)
        {
            analog_clock_scale += 0.1;
            sprintf(notification_msg, "Scale: %.1f", analog_clock_scale);
        }
        else
        {
            clock_sx += 0.1;
            clock_sy += 0.1;
            sprintf(notification_msg, "Scale: %.1f", clock_sx);
        }
        notification_timer = 800;
        do_draw = true;
    }
    if (kb_IsDown(kb_KeySub))
    {
        if (analog_clock)
        {
            analog_clock_scale -= 0.1;
            if (analog_clock_scale < 1)
                analog_clock_scale = 1;
            sprintf(notification_msg, "Scale: %.1f", analog_clock_scale);
        }
        else
        {
            clock_sx -= 0.1;
            clock_sy -= 0.1;
            if (clock_sx < 1)
                clock_sx = 1;
            if (clock_sy < 1)
                clock_sy = 1;
            sprintf(notification_msg, "Scale: %.1f", clock_sx);
        }
        notification_timer = 800;
        do_draw = true;
    }

    if (kb_JustPressed(kb_KeyYequ))
    {
        ampm_clock = !ampm_clock;
        sprintf(notification_msg, "12hr Mode: %s", ampm_clock ? "ON" : "OFF");
        notification_timer = 800;
        do_draw = true;
    }
    if (kb_JustPressed(kb_KeyWindow))
    {
        show_secs = !show_secs;
        sprintf(notification_msg, "Seconds: %s", show_secs ? "ON" : "OFF");
        notification_timer = 800;
        do_draw = true;
    }
    if (kb_JustPressed(kb_KeyZoom))
    {
        show_date = !show_date;
        sprintf(notification_msg, "Date: %s", show_date ? "ON" : "OFF");
        notification_timer = 800;
        do_draw = true;
    }
    if (kb_JustPressed(kb_KeyTrace))
    {
        analog_clock = !analog_clock;
        sprintf(notification_msg, "Mode: %s", analog_clock ? "Analog" : "Digital");
        notification_timer = 800;
        do_draw = true;
    }
    if (kb_JustPressed(kb_KeyGraph))
    {
        show_help = !show_help;
        do_draw = true;
        notification_timer = 0;
    }
    if (kb_JustPressed(kb_KeyEnter))
    {   
        show_help = !show_help;
        do_draw = true;
    }

    // Get current time
    boot_GetTime(&secs, &mins, &hrs);

    if (last_secs != secs)
    {
        do_draw = true;
        last_secs = secs;

    }
    

    if (do_draw)
        draw();

    return true;
}

int getTensDigit(int num)
{
    return abs(num / 10) % 10;
}
int getOnesDigit(int num)
{
    return abs(num % 10);
}
int getDayOfWeek(int day, int month, int year) {
    if (month < 3) {
        month += 12;
        year -= 1;
    }
    int K = year % 100;
    int J = year / 100;
    int h = (day + 13*(month+1)/5 + K + K/4 + J/4 + 5*J) % 7;
    // Convert to 0=Sunday, 1=Monday, ...
    int d = ((h + 6) % 7);  
    return d; 
}

void drawNumber(int8_t num, int x, int y, int8_t sx, int8_t sy)
{
    switch (num)
    {
    case 0:
        gfx_ScaledSprite_NoClip(digit0, x, y, sx, sy);
        break;
    case 1:
        gfx_ScaledSprite_NoClip(digit1, x, y, sx, sy);
        break;
    case 2:
        gfx_ScaledSprite_NoClip(digit2, x, y, sx, sy);
        break;
    case 3:
        gfx_ScaledSprite_NoClip(digit3, x, y, sx, sy);
        break;
    case 4:
        gfx_ScaledSprite_NoClip(digit4, x, y, sx, sy);
        break;
    case 5:
        gfx_ScaledSprite_NoClip(digit5, x, y, sx, sy);
        break;
    case 6:
        gfx_ScaledSprite_NoClip(digit6, x, y, sx, sy);
        break;
    case 7:
        gfx_ScaledSprite_NoClip(digit7, x, y, sx, sy);
        break;
    case 8:
        gfx_ScaledSprite_NoClip(digit8, x, y, sx, sy);
        break;
    case 9:
        gfx_ScaledSprite_NoClip(digit9, x, y, sx, sy);
        break;
    default:
        break;
    }
}
void drawColon(int x, int y, int8_t sx, int8_t sy)
{
    if (secs % 2 == 0)
    {
        gfx_ScaledSprite_NoClip(digitColon, x, y, sx, sy);
    }
    else
    {
        gfx_ScaledSprite_NoClip(digitBlankColon, x, y, sx, sy);
    }
}
void drawClock(int x, int y, float sx, float sy, bool center_anchor)
{
    uint8_t display_hours = hrs;
    if (ampm_clock)
    {
        if (hrs > 12)
            display_hours = hrs - 12;
        if (hrs == 0)
            display_hours = 12;
    }

    // All digits have same with so i use digit0_width
    int clock_digit_count = show_secs ? 8 : 5;
    int clock_w = sx * clock_digit_count * digit0_width;
    int clock_h = sy * digit0_height;
    if (center_anchor)
    {
        x = SCREEN_W_HALF - clock_w / 2;
        y = SCREEN_H_HALF - clock_h / 2;
    }

    int w = digit0_width * sx;
    drawNumber(getTensDigit(display_hours), x, y, sx, sy);
    drawNumber(getOnesDigit(display_hours), x + (w), y, sx, sy);
    drawColon(x + (2 * w), y, sx, sy);
    drawNumber(getTensDigit(mins), x + (3 * w), y, sx, sy);
    drawNumber(getOnesDigit(mins), x + (4 * w), y, sx, sy);
    if (show_secs)
    {
        drawColon(x + (5 * w), y, sx, sy);
        drawNumber(getTensDigit(secs), x + (6 * w), y, sx, sy);
        drawNumber(getOnesDigit(secs), x + (7 * w), y, sx, sy);
    }
}

static float min_cos[60];
static float min_sin[60];
static float h24_cos[24];
static float h24_sin[24];
static bool trig_initialized = false;

void drawAnalogClock(int x, int y, float s)
{
    if (!trig_initialized)
    {
        for (int i = 0; i < 60; i++)
        {
            float angle = i * (2 * 3.14159 / 60.0) - (3.14159 / 2.0);
            min_cos[i] = cos(angle);
            min_sin[i] = sin(angle);
        }
        for (int i = 0; i < 24; i++)
        {
            float angle = i * (2 * 3.14159 / 24.0) - (3.14159 / 2.0);
            h24_cos[i] = cos(angle);
            h24_sin[i] = sin(angle);
        }
        trig_initialized = true;
    }

    int hour_mod = ampm_clock ? 12 : 24;
    float angle_h = (hrs % hour_mod + mins / 60.0) * (2 * 3.14159 / (float)hour_mod) - (3.14159 / 2.0);
    float angle_m = (mins + secs / 60.0) * (2 * 3.14159 / 60.0) - (3.14159 / 2.0);
    float angle_s = (secs) * (2 * 3.14159 / 60.0) - (3.14159 / 2.0);

    int radius = 50 * s;
    gfx_SetColor(255);
    gfx_Circle(x, y, radius);

    // Ticks for hours (12 or 24)
    for (int i = 0; i < hour_mod; i++)
    {
        float c = (hour_mod == 12) ? min_cos[i * 5] : h24_cos[i];
        float s_val = (hour_mod == 12) ? min_sin[i * 5] : h24_sin[i];
        gfx_Line(x + c * (radius - 10), y + s_val * (radius - 10), x + c * (radius - 20), y + s_val * (radius - 20));
    }

    gfx_SetColor(15);
    // Ticks for minutes
    for (int i = 0; i < 60; i++)
    {
        if (!ampm_clock || (i % 5 != 0))
        {
            float c = min_cos[i];
            float s_val = min_sin[i];
            gfx_Line(x + c * (radius - 5), y + s_val * (radius - 5), x + c * (radius - 10), y + s_val * (radius - 10));
        }
    }

    // Hour hand
    gfx_Line(x, y, x + cos(angle_h) * (radius * 0.5), y + sin(angle_h) * (radius * 0.5));
    // Minute hand
    gfx_Line(x, y, x + cos(angle_m) * (radius * 0.8), y + sin(angle_m) * (radius * 0.8));
    // Second hand
    if (show_secs)
    {
        gfx_Line(x, y, x + cos(angle_s) * (radius * 0.9), y + sin(angle_s) * (radius * 0.9));
    }
    gfx_SetColor(0);
}
void draw(void)
{
    gfx_FillScreen(0);
    if (show_help) {
        gfx_SetTextFGColor(15);
        gfx_SetTextBGColor(0);
        gfx_SetTextScale(1, 1);
        gfx_PrintStringXY("----HELP----", 10, 10);
        gfx_PrintStringXY("F1: 12/24 Hour", 10, 20);
        gfx_PrintStringXY("F2: Seconds", 10, 30);
        gfx_PrintStringXY("F3: Date", 10, 40);
        gfx_PrintStringXY("F4: Analog/Digital", 10, 50);
        gfx_PrintStringXY("F5: Toggle Help", 10, 60);
        gfx_PrintStringXY("+/-: Scale Clock", 10, 70);
        gfx_PrintStringXY("CLEAR: Exit", 10, 80);
    }
    
    if (analog_clock)
    {
        drawAnalogClock(SCREEN_W_HALF, SCREEN_H_HALF, analog_clock_scale);
    }
    else
    {
        drawClock(0, 0, clock_sx, clock_sy, true);
    }
    if (show_date) {
        // e.g Tuesday, Feburary 3rd, 2026
        gfx_SetTextFGColor(15);
        gfx_SetTextBGColor(0);
        gfx_SetTextScale(1, 1);
        uint8_t day, month;
        uint16_t year;
        boot_GetDate(&day, &month, &year);
        int dow = getDayOfWeek(day, month, year);
        int postfix_idx = (day % 10 <= 3 && (day < 11 || day > 13)) ? (day % 10) : 0;

        char date_str[50];
        sprintf(date_str, "%s, %s %d%s, %d", days_of_week[dow], months_of_year[month - 1], day, date_postfixes[postfix_idx], year);
        
        
        int date_x = SCREEN_W_HALF - (gfx_GetStringWidth(date_str) / 2);
        gfx_PrintStringXY(date_str, date_x, 220);
    }

    if (notification_timer > 0) {
        gfx_SetTextScale(1, 1);
        gfx_SetTextFGColor(15);
        gfx_SetTextBGColor(0);
        gfx_PrintStringXY(notification_msg, 5, 5);
    }

    gfx_BlitBuffer();
}

int main(void)
{
    gfx_Begin();
    gfx_SetPalette(global_palette, sizeof_global_palette, 0);
    gfx_SetDrawBuffer();
    

    // Initialize previous keyboard state
    kb_Scan();
    for (int i = 1; i <= 7; i++)
    {
        prev_kb_Data[i] = kb_Data[i];
    }

    while (step())
    {
    }

    gfx_End();
    return 0;
}
