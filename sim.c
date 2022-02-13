/* Universal gravitation simulator */
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlocale.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <unistd.h>
#include <math.h>
#include "Flib/FillMask.h"
#include "Flib/FFont.h"

//定数
#define BORDER 2
#define WIDTH 1600
#define HEIGHT 900
#define I_WIDTH 600
#define I_HEIGHT 250
#define X_SIZE 60
#define BLACK 0x000000
#define WHITE 0xffffff
#define G 0.0000002

//構造体・グローバル変数定義
typedef struct{
  float x;
  float y;
}Coords;

typedef struct{
  int right;
  int left;
  int up;
  int down;
}Direction;

typedef struct{
  int r;
  int g;
  int b;
}Color;

typedef struct{
  float x;
  float y;
}Vector;

typedef struct{
  char name[256];
  int num;
  int mass;
  int radius;
  Vector accel;
  Vector velocity;
  Coords coords;
  Color color;
}Star;

typedef struct{
  char number[64];
  char mass[64];
  char radius[64];
  char accel_x[64];
  char accel_y[64];
  char velocity_x[64];
  char velocity_y[64];
  char coords_x[64];
  char coords_y[64];
}StarInfo;

//フォント系
FFont font;
FillMask mask = {1, 1, 1, 1, 3};

//プロトタイプ宣言
void updateCursor(XEvent, Coords*, Direction);
void DrawStar(Display*, Window, GC, Star*, Direction);
void DrawStars(Display*, Window, GC, Star*, int, Direction);
void OpenStarInfo(Display*, Window, Window);
void DrawStarInfo(Display*, Window, Window, GC, Star*, int, StarInfo);
void DrawInstructions(Display*, Window, GC);

//メイン関数
int main(int argc, char **argv){
  //変数宣言
  Display *dpy;
  Window w, info, exit;
  Window root;
  GC  gc;
  StarInfo starInfo;
  XTextProperty w_name;
  Pixmap pix;
  Pixmap pix_info;
  Coords cursor = {0, 0};
  Direction count = {0, 0, 0, 0};
  Coords center = {WIDTH / 2, HEIGHT / 2};

  int  screen, time, star_count;
  _Bool hasInfoOpened = 0, isHolding = 0; 
  int holdingNum = -1, clickedNum = -1;
  int frame = 0;
  int b_cursor[2] = {0, 0};
  float cursor_vx = 0.0;
  float cursor_vy = 0.0;

  dpy = XOpenDisplay("");
  root = DefaultRootWindow(dpy);
  screen = DefaultScreen(dpy);
  w = XCreateSimpleWindow(dpy, root, 100, 100, WIDTH, HEIGHT, BORDER, BLACK, BLACK);
  info = XCreateSimpleWindow(dpy, w, WIDTH - I_WIDTH - 10, 10, I_WIDTH, I_HEIGHT, BORDER, BLACK, 0xeeeeee);
  exit = XCreateSimpleWindow(dpy, info, I_WIDTH - X_SIZE - 7, 3, X_SIZE, X_SIZE, 1, BLACK, 0xdd4444);
  gc = XCreateGC(dpy, w, 0, NULL);
  pix = XCreatePixmap(dpy, w, WIDTH, HEIGHT, 24);
  pix_info = XCreatePixmap(dpy, info, WIDTH, HEIGHT, 24);

  //ウィンドウタイトル用
  w_name.value = "Universal gravitation simulator";
  w_name.encoding = XA_STRING;
  w_name.format = 8;
  w_name.nitems = strlen("Universal gravitation simulator");

  XSelectInput(dpy, w, ButtonPressMask | ButtonReleaseMask | Button3MotionMask | KeyPressMask);
  XSelectInput(dpy, exit, ButtonPressMask | ButtonReleaseMask);    
  setlocale(LC_ALL,"");
  XMapWindow(dpy, w);
  XSetWMProperties(dpy, w, &w_name, NULL, argv, argc, NULL, NULL, NULL);
  FLoadFont(&font, "Courier.font");

  //星を追加
  star_count = 7;

  Star sun = {"Sun", 0, 332837000, 75, {0.0, 0.0}, {0.0, 0.0}, {800, 450}, {255, 120, 20}};
  Star mercury = {"Mercury", 1, 55, 5, {0.0, 0.0}, {0.0, 0.52}, {550, 450}, {200, 200, 200}};
  Star venus = {"Venus", 2, 815, 15, {0.0, 0.0}, {0.0, -0.45}, {1130, 450}, {255, 40, 25}};
  Star earth = {"Earth", 3, 100, 20, {0.0, 0.0}, {0.42, 0.0}, {800, 850}, {0, 0, 255}};
  Star mars = {"Mars", 4, 107, 10, {0.0, 0.0}, {-0.4, 0.0}, {800,-5}, {255, 100, 10}};
  Star jupiter = {"Jupiter", 4, 31700, 40, {0.0, 0.0}, {0.02, 0.28}, {-100,450}, {238, 133, 63}};
  Star saturn = {"Saturn", 4, 9500, 32, {0.0, 0.0}, {-0.01, -0.21}, {2300, 450}, {205, 186, 150}};

  Star stars_init[256];
  stars_init[0] = sun;
  stars_init[1] = mercury;
  stars_init[2] = venus;
  stars_init[3] = earth;
  stars_init[4] = mars;  
  stars_init[5] = jupiter; 
  stars_init[6] = saturn;

  Star *stars[256];
  stars[0] = &sun;
  stars[1] = &mercury;
  stars[2] = &venus;
  stars[3] = &earth;
  stars[4] = &mars;
  stars[5] = &jupiter;
  stars[6] = &saturn;

  //メインループ
  while(1){
    //星の動きの計算
    for(int i = 0; i < star_count; i++){
      for(int j = i + 1; j < star_count; j++){
        float r, r2;
        //星の距離を算出
        r2 = (((*stars[i]).coords.x - (*stars[j]).coords.x) * ((*stars[i]).coords.x - (*stars[j]).coords.x)) + (((*stars[i]).coords.y - (*stars[j]).coords.y) * ((*stars[i]).coords.y - (*stars[j]).coords.y));
        r = sqrt(r2);
        //i → j
        (*stars[i]).accel.x = (((*stars[j]).coords.x - (*stars[i]).coords.x) / r) * (G * ((*stars[j]).mass / r2));
        (*stars[i]).accel.y = (((*stars[j]).coords.y - (*stars[i]).coords.y) / r) * (G * ((*stars[j]).mass / r2));
        (*stars[i]).velocity.x += (*stars[i]).accel.x;
        (*stars[i]).velocity.y += (*stars[i]).accel.y;
        //j → i
        (*stars[j]).accel.x = (((*stars[i]).coords.x - (*stars[j]).coords.x) / r) * (G * ((*stars[i]).mass / r2));
        (*stars[j]).accel.y = (((*stars[i]).coords.y - (*stars[j]).coords.y) / r) * (G * ((*stars[i]).mass / r2));
        (*stars[j]).velocity.x += (*stars[j]).accel.x;
        (*stars[j]).velocity.y += (*stars[j]).accel.y;
      }
    }
    //座標の確定
    for(int i = 0; i < star_count; i++){
      if(i == holdingNum){
        continue;
      }
      (*stars[i]).coords.x += (*stars[i]).velocity.x;
      (*stars[i]).coords.y += (*stars[i]).velocity.y;      
    }

    //背景を黒，文字を中央揃えに
    XSetForeground(dpy, gc, BLACK);
    XFillRectangle(dpy, pix, gc, 0, 0, WIDTH, HEIGHT);
    FSetLayout(LAYOUT_LEFT);

    //操作方法の表示
    DrawInstructions(dpy, pix, gc);

    //星の描画
    DrawStars(dpy, pix, gc, *stars, star_count, count);

    //星がクリックされたときに情報ウィンドウを描画
    if(hasInfoOpened == 1){
      DrawStarInfo(dpy, pix_info, exit, gc, *stars, clickedNum, starInfo);
    }

    //カーソルの変位を算出
    if(isHolding){
      cursor_vx = (cursor.x - b_cursor[0]) / 6;
      cursor_vy = (cursor.y - b_cursor[1]) / 6;
    }
    
    //マウスカーソルの座標を更新
    if(isHolding){
      b_cursor[0] = cursor.x;
      b_cursor[1] = cursor.y;
    }

    //イベント処理
    while(XEventsQueued(dpy, QueuedAlready) != 0){
      XEvent e;
      XNextEvent(dpy, &e);
      switch(e.type){
        case ButtonPress : {
          if(e.xbutton.button == 1){
            //すべての星に対して
            for(int i = 0; i < star_count; i++){
              //星の座標内をクリックしたら
              if(e.xbutton.x  - ((count.left - count.right) * 50) >= (*stars[i]).coords.x  - (*stars[i]).radius && e.xbutton.x  - ((count.left - count.right) * 50) <= (*stars[i]).coords.x + ((*stars[i]).radius * 2) - (*stars[i]).radius && e.xbutton.y - ((count.up - count.down) * 50) >= (*stars[i]).coords.y - (*stars[i]).radius && e.xbutton.y - ((count.up - count.down) * 50) <= (*stars[i]).coords.y + ((*stars[i]).radius * 2) - (*stars[i]).radius){
                //ウィンドウを開く
                OpenStarInfo(dpy, w, info);
                clickedNum = i;
                hasInfoOpened = 1;
              }  
            }
            //ウィンドウを閉じる
            if(e.xany.window == exit){
              XUnmapWindow(dpy, info);
              hasInfoOpened = 0;
            }
          }else if(e.xbutton.button == 3){
            //すべての星に対して
            for(int i = 0; i < star_count; i++){
              //星の座標内をクリックしたら
              if(e.xbutton.x  - ((count.left - count.right) * 50) >= (*stars[i]).coords.x - (*stars[i]).radius && e.xbutton.x  - ((count.left - count.right) * 50) <= (*stars[i]).coords.x + ((*stars[i]).radius * 2) - (*stars[i]).radius && e.xbutton.y - ((count.up - count.down) * 50) >= (*stars[i]).coords.y - (*stars[i]).radius && e.xbutton.y - ((count.up - count.down) * 50) <= (*stars[i]).coords.y + ((*stars[i]).radius * 2) - (*stars[i]).radius){
                holdingNum = i;
                isHolding = 1;
              }  
            }
          }
          break;
        }
        case ButtonRelease : {
          //離し判定
          if(isHolding){
            isHolding = 0;
            holdingNum = -1;
          }
          break;
        }
        case MotionNotify : {
          //星を投げる
          if(isHolding){
            updateCursor(e, &cursor, count);
            (*stars[holdingNum]).coords.x = cursor.x;
            (*stars[holdingNum]).coords.y = cursor.y;
            (*stars[holdingNum]).velocity.x = cursor_vx;
            (*stars[holdingNum]).velocity.y = cursor_vy;
          }
          break;
        }
        case KeyPress : {
          //ESCで終了
          if(e.xkey.keycode == 9){
            return 0;
          }
          if(e.xkey.keycode == 27){
            sun = stars_init[0];
            mercury = stars_init[1];
            venus = stars_init[2];
            earth = stars_init[3];
            mars = stars_init[4];
            jupiter = stars_init[5];
            saturn = stars_init[6];
            count.left = 0;
            count.right = 0;
            count.up = 0;
            count.down = 0;
          }          
          //矢印キー判定
          if(e.xkey.keycode == 113){
            count.left += 1;
          }
          if(e.xkey.keycode == 114){
            count.right += 1;
          }
          if(e.xkey.keycode == 111){
            count.up += 1;
          }
          if(e.xkey.keycode == 116){
            count.down += 1;
          }
        }
      }
    }
    //Pixmapからwへ描画
    XCopyArea( dpy, pix, w, gc, 0, 0, WIDTH, HEIGHT, 0, 0);
    XCopyArea( dpy, pix_info, info, gc, 0, 0, WIDTH, HEIGHT, 0, 0);
    //描画の確定
    XFlush(dpy);
    //μs単位で処理にディレイ
    usleep(5000); 
    frame += 1;   
  }
}

//操作説明の描画
void DrawInstructions(Display* dpy, Window w, GC gc){
  XSetForeground(dpy, gc, 0xbbbbbb);
  FDrawString(dpy, w, gc, font, "□", 15, 25, 2, 0);
  FFillMaskString(dpy, w, gc, font, "□", 15, 25, 2, 0, mask);
  FDrawString(dpy, w, gc, font, "Right click : Move the star", 70, 34, 1, 0);
  FFillMaskString(dpy, w, gc, font, "Right click : Move the star", 70, 34, 1, 0, mask);    
  FDrawString(dpy, w, gc, font, "〇", 15, 75, 2, 0);
  FFillMaskString(dpy, w, gc, font, "〇", 15, 75, 2, 0, mask);
  FDrawString(dpy, w, gc, font, "Left click : Show the star info", 70, 84, 1, 0);
  FFillMaskString(dpy, w, gc, font, "Left click : Show the star info", 70, 84, 1, 0, mask);
  FDrawString(dpy, w, gc, font, "×", 11, 125, 2, 0);
  FFillMaskString(dpy, w, gc, font, "×", 11, 125, 2, 0, mask);
  FDrawString(dpy, w, gc, font, "R Key : Reset all position", 72, 134, 1, 0);
  FFillMaskString(dpy, w, gc, font, "R Key : Reset all position", 72, 134, 1, 0, mask); 
  FDrawString(dpy, w, gc, font, "↑", 60, HEIGHT - 215, 2, 0);
  FFillMaskString(dpy, w, gc, font, "↑", 60, HEIGHT - 215, 2, 0, mask);
  FDrawString(dpy, w, gc, font, "←", 15, HEIGHT - 170, 2, 0);
  FFillMaskString(dpy, w, gc, font, "←", 15, HEIGHT - 170, 2, 0, mask);
  FDrawString(dpy, w, gc, font, "→", 105, HEIGHT - 170, 2, 0);
  FFillMaskString(dpy, w, gc, font, "→", 105, HEIGHT - 170, 2, 0, mask);
  FDrawString(dpy, w, gc, font, "↓", 60, HEIGHT - 125, 2, 0);
  FFillMaskString(dpy, w, gc, font, "↓", 60, HEIGHT - 125, 2, 0, mask);
  FDrawString(dpy, w, gc, font, "Arrow key : Move the viewpoint", 15, HEIGHT - 55, 1, 0);
  FFillMaskString(dpy, w, gc, font, "Arrow key : Move the viewpoint", 15, HEIGHT - 55, 1, 0, mask); 
}

//カーソルの座標を計算
void updateCursor(XEvent e, Coords *cursor, Direction count){
  (*cursor).x = e.xmotion.x - ((count.left - count.right) * 50);
  (*cursor).y = e.xmotion.y - ((count.up - count.down) * 50);
}

//星の描画処理
void DrawStar(Display* dpy, Window w, GC gc, Star *star, Direction count){
  FSetLayout(LAYOUT_CENTER);
  XSetForeground(dpy, gc, WHITE);
  FDrawString(dpy, w, gc, font, (*star).name, (*star).coords.x + ((count.left - count.right) * 50), (*star).coords.y - (*star).radius + ((count.up - count.down) * 50) - 60, 1, 0);
  FFillMaskString(dpy, w, gc, font, (*star).name, (*star).coords.x + ((count.left - count.right) * 50), (*star).coords.y - (*star).radius + ((count.up - count.down) * 50) - 60, 1, 0, mask);
  XSetForeground(dpy, gc, 0x010000 * (*star).color.r + 0x0100 * (*star).color.g + (*star).color.b);
  XFillArc(dpy, w, gc, ((*star).coords.x - (*star).radius) + ((count.left - count.right) * 50), (*star).coords.y - (*star).radius + ((count.up - count.down) * 50), (*star).radius * 2, (*star).radius * 2, 0, 360*64);
}

//すべての星を描画
void DrawStars(Display* dpy, Window w, GC gc, Star *stars, int star_count, Direction count){
  for(int i = 0; i < star_count; i++){
    DrawStar(dpy, w, gc, &stars[i], count);
  }
}

//情報ウィンドウを開く
void OpenStarInfo(Display* dpy, Window w, Window info){
  XMapSubwindows(dpy, w);
  XMapSubwindows(dpy, info);
}

//星の情報を描画
void DrawStarInfo(Display* dpy, Window info, Window exit, GC gc, Star *star, int num, StarInfo starInfo){
  //星の情報（数値）を char型に
  snprintf(starInfo.number, 256, "No.%d", star[num].num);
  snprintf(starInfo.mass, 256, "mass : %d", star[num].mass);
  snprintf(starInfo.radius, 256, "radius : %d", star[num].radius);
  snprintf(starInfo.velocity_x, 256, "velocity_x : %f", star[num].velocity.x);
  snprintf(starInfo.velocity_y, 256, "velocity_y : %f", star[num].velocity.y);
  snprintf(starInfo.coords_x, 256, "x : %f", star[num].coords.x);
  snprintf(starInfo.coords_y, 256, "y : %f", star[num].coords.y);

  //情報ウィンドウの表示
  XSetForeground(dpy, gc, 0x333333);
  XFillRectangle(dpy, info, gc, 0, 0, I_WIDTH, I_HEIGHT);
  XSetForeground(dpy, gc, WHITE);
  FDrawString(dpy, exit, gc, font, "X", X_SIZE / 2, 4, 2, 0);
  FFillMaskString(dpy, exit, gc, font, "X", X_SIZE / 2, 4, 2, 0, mask);

  //星の情報の表示
  FSetLayout(LAYOUT_LEFT);
  FDrawString(dpy, info, gc, font, star[num].name, 30, 15, 2, 0);
  FFillMaskString(dpy, info, gc, font, star[num].name, 30, 15, 2, 0, mask);
  XSetForeground(dpy, gc, 0x555555);
  FDrawString(dpy, info, gc, font, starInfo.number, I_WIDTH - X_SIZE - 100 , 40, 1, 0);
  XSetForeground(dpy, gc, 0xbbbbbb);
  FDrawString(dpy, info, gc, font, starInfo.mass, 50, 70, 1, 0);
  FDrawString(dpy, info, gc, font, starInfo.radius, 50, 95, 1, 0);
  FDrawString(dpy, info, gc, font, starInfo.velocity_x, 50, 115, 1, 0);
  FDrawString(dpy, info, gc, font, starInfo.velocity_y, 50, 140, 1, 0);
  FDrawString(dpy, info, gc, font, starInfo.coords_x, 50, 165, 1, 0);
  FDrawString(dpy, info, gc, font, starInfo.coords_y, 50, 190, 1, 0);
}
