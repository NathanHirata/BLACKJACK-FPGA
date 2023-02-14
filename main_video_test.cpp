#include "chu_init.h"
#include "gpio_cores.h"
#include "vga_core.h"
#include "sseg_core.h"
#include "ps2_core.h"
#include "string.h"
#include "xadc_core.h"

void test_start(GpoCore *led_p) {
   int i;

   for (i = 0; i < 20; i++) {
      led_p->write(0xff00);
      sleep_ms(50);
      led_p->write(0x0000);
      sleep_ms(50);
   }
}

void frame_check(FrameCore *frame_p) {
   int x, y, color;

   frame_p->bypass(0);
   for (int i = 0; i < 10; i++) {
      frame_p->clr_screen(0x008);  // dark green
      for (int j = 0; j < 20; j++) {
         x = rand() % 640;
         y = rand() % 480;
         color = rand() % 512;
         frame_p->plot_line(400, 200, x, y, color);
      }
      sleep_ms(50);
   }
   sleep_ms(100);
}

void cardSpriteDeal(SpriteCore *ghost_p, int x, int y, int j){
	for (int i = 0; i < 87-(j*8); i++) {
		   ghost_p->move_xy(x+10, y);
		   sleep_ms(15);
		   x = x - 4;
	}
}

void write2disp(OsdCore *osd_p, int x, int y, char word[]){
	for (int i = 0; i < strlen(word); i++) {
		osd_p->wr_char((x-(strlen(word)/2)) + i, y, word[i]);
	 }
}

void update_osd(OsdCore *osd_p, int mon, int bets) {
   osd_p->set_color(0xfff, 0x000); // dark gray/green
   osd_p->bypass(0);
   osd_p->clr_screen();

   write2disp(osd_p, 37, 7, "DEALER HAND");
   write2disp(osd_p, 36, 15, "YOUR HAND");

   write2disp(osd_p, 11, 2, "Player Money: ");
   write2disp(osd_p, 6, 4, "Bet: ");

	int m[] = {36,(mon/1000)+48,(mon%1000/100)+48,(mon%100/10)+48,(mon%10)+48};
	int b[] = {36,(bets/100)+48,(bets%100/10)+48,(bets%10)+48};

	for(int i = 0; i<5; i++){osd_p->wr_char(18+i, 2, m[i]);}
	for(int i = 0; i<4; i++){osd_p->wr_char(9+i, 4, b[i]);}
}

void update_money(OsdCore *osd_p, int bets,int winlose) {
	int sign;
	if(winlose)
		sign = 43;
	else
		sign = 45;

	int b[] = {sign,36,(bets/100)+48,(bets%100/10)+48,(bets%10)+48};

	for(int i = 0; i<5; i++){osd_p->wr_char(18+i, 3, b[i]);}
}

int check_win(OsdCore *osd_p, int dealer, int player) {
	int decide = 0;
	if(player == 21 && dealer != 21){ //player wins if 21 is drawn
		write2disp(osd_p, 40, 53, "YOU WIN!");
		decide = 2;
	 }
	else if(player > 21){ //lose
		write2disp(osd_p, 40, 53, "YOU BUSTED!");
		 decide = 1;
	 }
	 else if(dealer>21){ // win
		 write2disp(osd_p, 40, 53, "DEALER BUSTED!");
		 decide = 2;
	 }
	 else if(dealer>player){ // lose
		 write2disp(osd_p, 40, 53, "DEALER WINS!");
		 decide = 1;

	 }
		 return decide;
}

int placeBet(XadcCore *adc_p, Ps2Core *ps2_p,OsdCore *osd_p, int money){
	double v;
	int b;
	char ch;
	write2disp(osd_p, 40, 56, "Press 3 to Enter Bet");
    while(1){
       v = adc_p->read_adc_in(0);
       b = ((int)(v*1000));
       if(b>money) b = money;
       else if(b<5) b = 5;
       else if (b>999) b = 999;
       update_osd(osd_p, money, b);
	   if (ps2_p->get_kb_ch(&ch)) {
		   if (ch == 51){// place bet 3
			   break;
		   }
	   }
	   sleep_ms(5);
   }
    return b;
}

int ps2_check(Ps2Core *ps2_p,OsdCore *osd_p, int state, int money, int bet) {
   int id;
   char ch;
   int checkbutton = 1;
   int KeyVal=0;

   uart.disp("\n\rPS2 device (1-keyboard / 2-mouse): ");
   id = ps2_p->init();
   uart.disp(id);
   uart.disp("\n\r");


   if(state == 0){
	   write2disp(osd_p, 40, 56, "Press SpaceBar to Start");
	   while(checkbutton){
		   if (ps2_p->get_kb_ch(&ch)) {
				 if (ch == 32){// start spacebar
					 KeyVal = 1;
					 checkbutton=0;
				 }
		   }
	   }
   }
   else if(state == 1){
	   write2disp(osd_p, 40, 56, "Press 1 to hit   |   Press 2 to Stay");
	   while(checkbutton){
		   if (ps2_p->get_kb_ch(&ch)) {
			   if (ch == 49){//hit 1
				   KeyVal = 2;
				   checkbutton=0;
			   }
			   else if (ch == 50){
				   KeyVal = 3;//stay 2
				   checkbutton=0;
			   }
		   }
	   }
   }
   else if (state == 2){
	   write2disp(osd_p, 40, 56, "Press 4 to Play Again");
	   while(checkbutton){
		   if (ps2_p->get_kb_ch(&ch)) {
			   if (ch == 52){// play again 4
				   KeyVal = 4;
				   checkbutton=0;
			   }
		   }
	   }
   }
   /*else if (state == 3){
	   write2disp(osd_p, 40, 56, "Press 3 to Enter Bet");
   	   while(checkbutton){
   		   if (ps2_p->get_kb_ch(&ch)) {
   			   if (ch == 51){// place bet 3
   				   KeyVal = 6;
   				checkbutton=0;
   			   }
   		   }
   	   }
    }*/
   else if (state == 4){
   	   write2disp(osd_p, 40, 56, "Press 4 to Reset Game");
   	   while(checkbutton){
   		   if (ps2_p->get_kb_ch(&ch)) {
   			   if (ch == 53){// restart game 5
   				   KeyVal = 5;
   				   checkbutton=0;
   			   }
   		   }
   	   }
      }
   update_osd(osd_p,money, bet);
   uart.disp("\n\rExit PS2 test \n\r");
   return KeyVal;
}

void cardMath(CardCore *card_p, int x,int y, int* total, int* aces){
	   int cardFace;
	   int card = rand() % 13+1;
	   if(card > 10) cardFace = 10;
	   else if(card == 1) {cardFace = 11;++(*aces);}
	   else cardFace = card;
	   *total += cardFace;
	   if(*aces>0){
		   if(*total>21){
			   *total-=(10*(*aces));
			   --(*aces);
		   }
	   }
	   card_p->wr_char(x, y, card);
	   sleep_ms(200);
}

void ghost_check(SpriteCore *ghost_p,SpriteCore *backCard_p, CardCore *card_p, Ps2Core *ps2_p,OsdCore *osd_p,XadcCore *adc_p) {
    int h1;
	int bet=5, money=1000;
	int checkW;
	int playagain=1,stay=0;
	int adcVal;

	update_osd(osd_p, money, bet);
    card_p->bypass(0);
   	card_p->clr_screen();
    ghost_p->bypass(0);
    ghost_p->move_xy(590, 128);

    // first 2 cards
    while(playagain){
    	int dealertotal=0,playertotal=0;
    	int aceD=0,aceP=0;
    	bet=5;
    	stay=0;
    	card_p->clr_screen();

    	if(ps2_check(ps2_p,osd_p, 0, money, bet) == 1){	// checks for keyboard 'space'

			/*if(ps2_check(ps2_p,osd_p, 3 , money, bet) == 6){ //betting
				bet = 200; //adcVal; if (bet > money) bet = money;
			}	*/		   //        if (bet < 5) bet = 5;

			bet = placeBet(adc_p, ps2_p, osd_p, money);

			for (int j = 0; j < 2; j++) {	// 'deals two cards each'
				ghost_p->bypass(0);
				cardSpriteDeal(ghost_p, 590, 256, j);
				cardMath(card_p, j+8, 4, &playertotal, &aceP);
				cardSpriteDeal(ghost_p, 590, 128, j);
				cardMath(card_p, j+8, 2, &dealertotal, &aceD);
			}

			ghost_p->move_xy(288, 128); // move sprite out of play
			backCard_p->bypass(0);
			backCard_p->move_xy(288, 128);

			if(playertotal == 21 && dealertotal != 21){ //player wins if 21 is drawn
				ghost_p->bypass(1);
				backCard_p->bypass(1);
				money+=bet;
				write2disp(osd_p, 40, 53, "YOU WIN!");
				update_money(osd_p, bet,1);
				stay=1;
			}
			else if(dealertotal == 21 && playertotal != 21){ // dealer wins if 21 is drawn
				ghost_p->bypass(1);
				backCard_p->bypass(1);
				money-=bet;
				write2disp(osd_p, 40, 53, "DEALER WINS!");
				update_money(osd_p, bet,0);
				stay=1;
			}
    	}

    	h1=2; // position of next cards

    	while(!stay){// waits until player stays
    		int hitstay = ps2_check(ps2_p,osd_p, 1, money, bet);

    		if(hitstay == 2){ // hit 1 card
    		   cardSpriteDeal(ghost_p, 590, 256, h1);
    		   cardMath(card_p, h1+8, 4, &playertotal, &aceP);
    		   ghost_p->move_xy(590, 128);
    		   h1++;

    		   if(playertotal>21){
    			   backCard_p->bypass(1);
    			   money-=bet;
    			   check_win(osd_p, dealertotal, playertotal);
    			   update_money(osd_p, bet,0);
    			   stay=1;
    		   }
    		   if(h1==5)stay=1;
    	   	}

    	   	else if (hitstay == 3){ // stay
    	   		backCard_p->bypass(1);
    	   		for (int j = 0; j < 3; j++) {
					   if(check_win(osd_p, dealertotal, playertotal) == 1){
						   ghost_p->move_xy(590, 128);
						   money-=bet;
						   update_money(osd_p, bet,0);
						   stay=1;
						   break;
					   }

    				   cardSpriteDeal(ghost_p, 590, 128, j+2);
    				   cardMath(card_p, j+10, 2, &dealertotal, &aceD);
    				   ghost_p->move_xy(590, 128);
    				   stay=1;

    				   checkW = check_win(osd_p, dealertotal, playertotal);

    				   if(checkW == 1){
    					   money-=bet;
    					   update_money(osd_p, bet,0);
    					   break;
    				   }
    				   else if(checkW == 2){
    					   money+=bet;
    					   update_money(osd_p, bet,1);
    					   break;
    				   }
    			  }
			  }
			  if(dealertotal==playertotal){ //
					write2disp(osd_p, 40, 53, "PUSH!");
			  }
    	}
    	if(money<0){
    		if(ps2_check(ps2_p,osd_p, 4, money, bet) == 5){
				playagain=0;
			}
    	}
    	else if(ps2_check(ps2_p,osd_p, 2, money, bet) == 4){
    		playagain=1;
    	}
    }
    sleep_ms(100);
}

// external core instantiation
GpoCore led(get_slot_addr(BRIDGE_BASE, S2_LED));
GpiCore sw(get_slot_addr(BRIDGE_BASE, S3_SW));
FrameCore frame(FRAME_BASE);
GpvCore bar(get_sprite_addr(BRIDGE_BASE, V7_BAR));
GpvCore gray(get_sprite_addr(BRIDGE_BASE, V6_GRAY));
SpriteCore ghost(get_sprite_addr(BRIDGE_BASE, V3_GHOST), 4096);
SpriteCore mouse(get_sprite_addr(BRIDGE_BASE, V1_MOUSE), 1024);
SpriteCore title(get_sprite_addr(BRIDGE_BASE, V5_USER5), 1024);
OsdCore osd(get_sprite_addr(BRIDGE_BASE, V2_OSD));
CardCore card(get_sprite_addr(BRIDGE_BASE, V4_USER4));
SsegCore sseg(get_slot_addr(BRIDGE_BASE, S8_SSEG));
Ps2Core ps2(get_slot_addr(BRIDGE_BASE, S11_PS2));
XadcCore adc(get_slot_addr(BRIDGE_BASE, S5_XDAC));

int main() {
	srand(0);
   while (1) {
      test_start(&led);
      // bypass all cores
      frame.bypass(1);
      bar.bypass(1);
      gray.bypass(1);
      ghost.bypass(1);
      osd.bypass(1);
      card.bypass(1);
      mouse.bypass(1);
      title.bypass(1);
      sleep_ms(100);
      title.move_xy(150,12);
      title.bypass(0);
      // enable cores one by one
      frame_check(&frame);

      ghost_check(&mouse, &ghost, &card, &ps2, &osd, &adc);

   } // while
} //main
