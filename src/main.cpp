//Arduino-TFT_eSPI board-template main routine. There's a TFT_eSPI create+flush driver already in LVGL-9.1 but we create our own here for more control (like e.g. 16-bit color swap).

#include <lvgl.h>
#include <TFT_eSPI.h>
#include <ui.h>
#include "encoder.h"


/*Don't forget to set Sketchbook location in File/Preferences to the path of your UI project (the parent foder of this INO file)*/

/*Change to your screen resolution*/
static const uint16_t screenWidth  = 280;
static const uint16_t screenHeight = 240;

//button
lv_group_t *gp1;    // 新增全局变量gp，用于创建组
lv_group_t *gp2;    // 新增全局变量gp2，用于创建组
lv_indev_t* indev; //创建KeyBoard结构体
uint8_t D0 = 16; //定义KEY1和KEY2的引脚
uint8_t D1 = 17;
bool RefreshCurrentIndex=false;//控制电流检测数据是否刷新

//chart
lv_chart_series_t * ui_Chart1_series_2 ;
lv_coord_t ui_Chart1_series_2_array[] = { 0,0, 0, 0, 0, 0, 0, 0, 0, 0 };//初始化为0


//test
int num=0;
int index1=1;
float Current = 0;//实测电流
float DarkCurrent = 0;//暗电流电流
float LightCurrent = 0;//光电流电流
//parameter of mine
int16_t WidthOfText=0;
byte TxCmd[5]={0x55,0x55,0x01,0x01,0xac};
//定义Serial1的收发引脚
int8_t TX_Pin=18;
int8_t RX_Pin=19;


enum { SCREENBUFFER_SIZE_PIXELS = screenWidth * screenHeight / 10 };
static lv_color_t buf [SCREENBUFFER_SIZE_PIXELS];

TFT_eSPI tft = TFT_eSPI( screenWidth, screenHeight ); /* TFT instance */

#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(const char * buf)
{
    Serial.printf(buf);
    Serial.flush();
}
#endif

/* Display flushing */
void my_disp_flush (lv_display_t *disp, const lv_area_t *area, uint8_t *pixelmap)
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    if (LV_COLOR_16_SWAP) {
        size_t len = lv_area_get_size( area );
        lv_draw_sw_rgb565_swap( pixelmap, len );
    }

    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors( (uint16_t*) pixelmap, w * h, true );
    tft.endWrite();

    lv_disp_flush_ready( disp );
}


/*Set tick routine needed for LVGL internal timings*/
static uint32_t my_tick_get_cb (void) { return millis(); }

//button

// void my_keypad_read(lv_indev_t *indev_driver, lv_indev_data_t *data)    // 新增函数my_keypad_read：回调函数用于读取按键状态和按键值
// {
//     if(digitalRead(D0) == LOW) {    // 读取D0是否被按下（实际高低由实际按键接线为准），未按下时是拉高，按下是拉低
//         data->state = LV_INDEV_STATE_PR;    // 定义按键状态是【press/按下】
//         data->key = LV_KEY_NEXT;            // 设置值为LV_KEY_NEXT，D0定义为【聚焦切换】功能按键
//     }else if(digitalRead(D1) == LOW){      // 读取D1是否被按下
//         data->state = LV_INDEV_STATE_PR;    // 定义按键状态是【press/按下】
//         data->key = LV_KEY_ENTER;           // 设置值为LV_KEY_ENTER，D1定义为【确定】功能按键
//     }
//     else {
//         data->state = LV_INDEV_STATE_REL;    // 定义按键状态是【release/松开】
//     }
// }


// void set_button(void){                                // 新增set_button函数：创建输入设备组函数
//   static lv_indev_t* indev;
//   indev = lv_indev_create();                        // 创建一个输入设备(indev)
//   lv_indev_set_type(indev, LV_INDEV_TYPE_KEYPAD);    // 该输入设备类型定义为键盘(KEYPAD)
//   lv_indev_set_read_cb(indev, my_keypad_read);       // 键盘绑定回调函数my_keypad_read，回调函数读取键盘状态和按键值
//   gp = lv_group_create();                            // 创建按键组
//   lv_group_set_default(gp);                          // 设为默认组（实际开发可能多个设备组，这里就一个组）
//   lv_group_add_obj(gp, ui_Button1);                  // 将按键组件1和按键组件2分别加入按键组
//   lv_group_add_obj(gp, ui_Button2);
//   lv_indev_set_group(indev, gp);                    // 将输入设备和按键组绑定。
// }



//encoder
 
void my_encoder_read(lv_indev_t *indev_driver, lv_indev_data_t *data)    // 新增函数my_encoder_read：回调函数用于读取旋转编码器状态
{
    int16_t direction = encoder_get_diff();
    if(encoder_get_is_push()==true){
         data->state = LV_INDEV_STATE_PR;
         data->key = LV_KEY_ENTER;
    }else if(direction == 1){/* 编码器转动方向，0为静止，1为顺时针旋转，-1为逆时针旋转 */
        Serial.printf("Diff is 1\r\n");
        data->state = LV_INDEV_STATE_PR;
        data->key = LV_KEY_NEXT;
    }else if(direction == -1){
         Serial.printf("Diff is -1\r\n");
        data->state = LV_INDEV_STATE_PR;
        data->key= LV_KEY_PREV;
    }else{
        data->state = LV_INDEV_STATE_REL;
    }
}


void set_encoder(void){                                // 新增set_button函数：创建输入设备组函数
  indev = lv_indev_create();                        // 创建一个输入设备(indev)
  lv_indev_set_type(indev, LV_INDEV_TYPE_KEYPAD);    // 该输入设备类型定义为键盘(KEYPAD)
  //将实体控件与UI控件绑定
  lv_indev_set_read_cb(indev, my_encoder_read);       // 键盘绑定回调函数my_keypad_read，回调函数读取键盘状态和按键值
  gp1 = lv_group_create();                            // 创建按键组
  //lv_group_set_default(gp);                          // 设为默认组（实际开发可能多个设备组，这里就一个组）
  //screen1
  lv_group_add_obj(gp1, ui_Button1);                  // 将按键组件1和按键组件3分别加入按键组
  lv_group_add_obj(gp1, ui_Button3);
    lv_indev_set_group(indev, gp1);                    // 初始状态，先将输入设备和Screen1按键组绑定。
  
    //screen2
   gp2 = lv_group_create();  

  lv_group_add_obj(gp2, ui_Button4);                  // 将按键组件1和按键组件3分别加入按键组
  lv_group_add_obj(gp2, ui_Button5);
  
}

void RefreshCurrentData()//用于实时更新电流数据
{
    if (RefreshCurrentIndex == true)
    {
        Serial1.write(TxCmd, 5);
        delay(50);



        if (Serial1.available() > 0)
        { // 用于读取多字节，并进行数据处理之后显示，同时更新曲线
            uint8_t receiveData[64];
            int32_t RawData;
            
            delay(10);

            // 读取可用的字节数
            int numBytes = Serial1.available();
            Serial1.read(receiveData, numBytes); // receiveData中的数据是以ASCII码的形式，如串口接收1，则receiveDatap[0]=49
            //对接收的数据进行数据处理
            RawData=(receiveData[4]<<24)|(receiveData[5]<<16)|(receiveData[6]<<8)|receiveData[7];//按位与预算，将32位补码转换成带符号整数
            Current=RawData*0.1;
            //在屏幕上显示
            lv_label_set_text_fmt(ui_Label10, "%.1f", Current);//实测电流
            Serial.printf("Received RawData:%.1f\r\n",Current);
            LightCurrent=Current-DarkCurrent;
            lv_label_set_text_fmt(ui_Label9, "%.1f", LightCurrent);//光电流
            // //更新曲线chart
            // lv_chart_set_next_value(ui_Chart1, ui_Chart1_series_1, (lv_coord_t)Current); // 更新曲线
        }
    }
}

void setup ()
{
    pinMode(D0, INPUT);  // 定义为【聚焦切换】按键
    pinMode(D1, INPUT);  // 定义为【确定】按键
    Serial.begin( 115200 ); /* prepare for possible serial debug */

    String LVGL_Arduino = "Hello Arduino! ";
    LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
    Serial1.begin(9600,SERIAL_8N1,RX_Pin,TX_Pin);

    Serial.println( LVGL_Arduino );
    Serial.println( "I am LVGL_Arduino" );

    lv_init();

#if LV_USE_LOG != 0
    lv_log_register_print_cb( my_print ); /* register print function for debugging */
#endif

    tft.begin();          /* TFT init */
    tft.setRotation( 1 ); /* Landscape orientation, flipped */

    static lv_disp_t* disp;
    disp = lv_display_create( screenWidth, screenHeight );
    lv_display_set_buffers( disp, buf, NULL, SCREENBUFFER_SIZE_PIXELS * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL );
    lv_display_set_flush_cb( disp, my_disp_flush );

    lv_tick_set_cb( my_tick_get_cb );

    ui_init();
    // set_button(); // 一定在ui_init()函数后面调用set_button()函数创建输入设备组，因为按键组件在ui_init()中才被初始化。
    //绑定Chart的数据
    //chart
    ui_Chart1_series_2 = lv_chart_add_series(ui_Chart1, lv_color_hex(0x808080), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_ext_y_array(ui_Chart1, ui_Chart1_series_2, ui_Chart1_series_2_array);


    encoder_config();//编码器配置
    input_task_create();
    set_encoder();

    Serial.println( "Setup done" );


}

void loop ()
{
    lv_timer_handler(); /* let the GUI do its work */
    RefreshCurrentData();
    Serial.printf("RefreshCurrentIndex:%d\r\n",RefreshCurrentIndex);
    if (RefreshCurrentIndex == true)
    {
        lv_chart_set_next_value(ui_Chart1, ui_Chart1_series_2, (lv_coord_t)Current); 
         //测试chart更新
         
        //  if(num>25)
        //  {
        //     index1=-1;
        //  }else if(num<-25)
        //  {
        //     index1=1;
        //  }
        //  num=num+index1;
        //  Serial.printf("num:%d\r\n",num);
        //  lv_chart_set_next_value(ui_Chart1, ui_Chart1_series_2, num); // 更新曲线
        
    }
    delay(5);
}
