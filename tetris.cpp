#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <sstream>
using namespace std;

static const int tilesize = 30;
static const int bgcolor=97;
static const int xmaxtiles= 14;
static const int ymaxtiles= 20;
static const double timeout = 0.5;

class Tile {
	public:
	int x; int y;
};
//functions to move and rotate entire tileset of 4; functions called in move functions of ActivePiece class
void ts_down (Tile ts[4]) { for (int i=0; i<4; i++) ts[i].y++;}
void ts_right(Tile ts[4]) { for (int i=0; i<4; i++) ts[i].x++;}
void ts_left (Tile ts[4]) { for (int i=0; i<4; i++) ts[i].x--;}
void ts_copy (Tile in[4], Tile out[4]) { 
	for (int i=0; i<4; i++){
		out[i].x=in[i].x;
		out[i].y=in[i].y;
	}
}
void ts_rotate(Tile ts[4], int centerTile) {
	if(centerTile!=-1)
	{	int x[4], y[4];
		for( int i=0; i<4; i++)
		{	x[i]=ts[centerTile].x-ts[i].x;
			y[i]=ts[centerTile].y-ts[i].y;
		}
		for( int i=0; i<4; i++ )
		{	ts[i].x=ts[centerTile].x+y[i];
			ts[i].y=ts[centerTile].y-x[i];
		}
	}
}

class Board;
class ActivePiece;
//class board derived from Fl_Widget
class Board : public Fl_Widget
{
	//occup[][] stores color of every tile on board, so occup[i][j]=97 means blank tile
	int occup[xmaxtiles][ymaxtiles];
	ActivePiece *p;
	char* scoreLabel;
	Fl_Box *scorebox; 
	int score;
	int bx, by, bw, bh;
	int color;

public:	
	Board(int bx, int by, int bw, int bh, int c) : Fl_Widget (bx,by,bw,bh) { 
		this->bx = bx; this->by = by; this->bw = bw; this->bh = bh; color =c;}

	Board();
	void setScoreBox(Fl_Box** sb);		//changes the score on row completion by 50
	bool isFree(Tile ts[4]);			//checks if place for tileset[4] is free on board
	bool inBoard(Tile ts[4]); 			// keeps the shapes inside the board
	bool transferTiles(Tile ts[4], int color);	//drops ActivePiece on the board
	void deleteRow(int row);		//called in rowcheck to remove filled rows
	bool rowcheck(int row);			//checks after each move for completed rows
	void draw();					
	int handle(int e);				
	bool gameend();					//if no place for ActivePiece then game ends
	int periodic();					//function making call to all functions regularly at timeouts
};

class ActivePiece{
private:
	friend class Board;				//uses functionalities of board like isFree, inBoard
	Tile tileSet[4];
	int color;
	int midpointTile;				//tile about which rotation takes place
	Board *b;						//to keep a track of current tiles on board
	
	ActivePiece(Board *container);
	//checks for validity of move and makes call to respective ts_move functions
	bool moveDown ();		
	bool moveLeft ();		
	bool moveRight ();		
	bool rotate();			
};

void timeractions(void *b) {
	((Board *)b)->periodic ();
}

Board::Board():Fl_Widget (0,0,xmaxtiles*tilesize,
			ymaxtiles*tilesize,"Tetris")
{	//creates the first piece
	p = new ActivePiece(this);
	//initializing the board
	for(int i=0; i<xmaxtiles; i++)
		for(int j=0; j<ymaxtiles; j++)
			occup[i][j]=bgcolor;
}
bool Board::gameend()
{	//checks if new piece can be placed or not
	for(int i=0; i<4; i++)
		if(occup[ p->tileSet[i].x ][ p->tileSet[i].y ]!=97) return true;
		else return false; 
}
int Board::periodic() {
	if( !p->moveDown() ){
		//if picece cannot move further then this part is executed
		transferTiles(p->tileSet,p->color);
		p= new ActivePiece(this);
		for(int i=0; i<20; i++)
			rowcheck(i);
			if(gameend()) 
			{
				cout<<"Game Over ! "<<endl; 
				exit(0);
			}		
	}	
	redraw();
	Fl::repeat_timeout(timeout, timeractions, this);
}
bool Board::rowcheck(int i)
{
	for( int j=0; j<14; j++) if(occup[j][i]==97) return false;
	deleteRow(i);
	return true;
}
void Board::deleteRow(int i)
{	//shifts every row above ith row, a step below
	for(i; i>0; i--)
		for(int j=0; j<14; j++)
			occup[j][i]=occup[j][i-1];
	//sets top row to bgcolor
	for(int j=0; j<14; j++) occup[j][0]=97;
	//increases the score here
		score+=1;
		stringstream strs;
		strs << score;
		string temp_str = strs.str();
		strcpy(scoreLabel,"Score: ");
		strcat(scoreLabel,(char*) temp_str.c_str() );
		scorebox->label(scoreLabel); 	// void label(char *) itself calls the redraw() for that object.				
}
bool Board::transferTiles(Tile ts[4], int color)
{	//making board color same as that of piece
	for(int i=0; i<4; i++) 
	{
		occup[ts[i].x][ts[i].y]=color;
	}
}
bool Board::inBoard(Tile ts[4])
	{
		for (int i=0; i<4; i++) if( (ts[i].x<0) || (ts[i].x>13) || (ts[i].y<0) || (ts[i].y>19) ) return false;
		return true;
	}
bool Board::isFree(Tile ts[4])
	{
		for (int i=0; i<4; i++) if(occup[ts[i].x][ts[i].y]!=bgcolor) return false;
		return true;
	}

void Board::setScoreBox(Fl_Box** sb)
{
	scorebox = *sb ;
	score=0;
	scoreLabel = (char*) malloc(sizeof(char)*10);
}
void Board::draw()
{
	for(int i=0; i<xmaxtiles; i++)
		{
			for(int j=0; j<ymaxtiles; j++)
			{
				fl_draw_box(FL_BORDER_BOX,i*tilesize,j*tilesize,
						tilesize,tilesize,occup[i][j]);
			}
		}
	for(int i=0; i<4; i++)
	{
			fl_draw_box(FL_BORDER_BOX, p->tileSet[i].x*tilesize, p->tileSet[i].y*tilesize, tilesize, tilesize, p->color);
	}	
}
int Board::handle(int e)
	{
		if(e==8 || e==12)
		switch (Fl::event_key())
		{
		//to make the game more user friendly, down key takes the piece down and only up key rotates the shape
		case 65361: p->moveLeft(); break;		//left key
		case 65362: p->rotate(); break;			//up key
		case 65363: p->moveRight(); break;		//right key
		case 65364: while(p->moveDown()){} break;	//down key
		}
		redraw();
	}

ActivePiece::ActivePiece(Board *container)
{
	int r = rand()%7;
	int c = (int) (xmaxtiles/2); //xcenter
	b = container;
	color = rand()%96+1;
	switch(r) {
		//different cases draw the different shapes generallly used
		case 0:
		tileSet[0].x=c-2; tileSet[0].y=0; 
		tileSet[1].x=c-1; tileSet[1].y=0;
		tileSet[2].x=c+1; tileSet[2].y=0;
		tileSet[3].x=c; tileSet[3].y=0;
		midpointTile = 3;
		break;
		case 1:
		tileSet[0].x=c; tileSet[0].y=0;  
		tileSet[1].x=c-1; tileSet[1].y=0;
		tileSet[2].x=c+1; tileSet[2].y=0;
		tileSet[3].x=c; tileSet[3].y=1;
		midpointTile = 3;
		break;
		case 2:
		tileSet[0].x=c-1; tileSet[0].y=1; 
		tileSet[1].x=c-1; tileSet[1].y=0;
		tileSet[2].x=c+1; tileSet[2].y=0;
		tileSet[3].x=c; tileSet[3].y=0;
		midpointTile = 3;
		break;
		case 3:
		tileSet[0].x=c+1; tileSet[0].y=1; 
		tileSet[1].x=c-1; tileSet[1].y=0;
		tileSet[2].x=c+1; tileSet[2].y=0;
		tileSet[3].x=c; tileSet[3].y=0;
		midpointTile = 3;
		break;
		case 4:
		tileSet[0].x=c-1; tileSet[0].y=0; // 
		tileSet[1].x=c-1; tileSet[1].y=1;
		tileSet[2].x=c; tileSet[2].y=1;
		tileSet[3].x=c; tileSet[3].y=0;
		midpointTile = -1;     //no rotation needed for square
		break;
		case 5:
		tileSet[0].x=c-1; tileSet[0].y=0; 
		tileSet[1].x=c-1; tileSet[1].y=1;
		tileSet[2].x=c; tileSet[2].y=2;
		tileSet[3].x=c; tileSet[3].y=1;
		midpointTile = 3;
		break;
		case 6:
		tileSet[0].x=c-1; tileSet[0].y=1;  
		tileSet[1].x=c-1; tileSet[1].y=2;
		tileSet[2].x=c; tileSet[2].y=0;
		tileSet[3].x=c; tileSet[3].y=1;
		midpointTile = 3;
		break;
	}	
}	
bool ActivePiece::rotate()
{
	Tile proposedSet[4];
	//makes a copy of given piece
	ts_copy(tileSet, proposedSet);
	//makes changes to the copy
	ts_rotate(proposedSet,midpointTile);
	if (b->isFree(proposedSet) && b->inBoard(proposedSet)) 
	{	//if change allowed then copies the changed location to original one
		ts_copy(proposedSet, tileSet);
		return true;
	}
	else return false;	
}
bool ActivePiece::moveRight ()
{
	Tile proposedSet[4];
	ts_copy(tileSet, proposedSet);
	ts_right(proposedSet);
	if (b->isFree(proposedSet) && b->inBoard(proposedSet)) 
	{
		ts_copy(proposedSet, tileSet);
		return true;
	}
	else return false;
}
bool ActivePiece::moveLeft ()
{
	Tile proposedSet[4];
	ts_copy(tileSet, proposedSet);
	ts_left(proposedSet);
	if (b->isFree(proposedSet) && b->inBoard(proposedSet)) 
	{
		ts_copy(proposedSet, tileSet);
		return true;
	}
	else return false;
}
bool ActivePiece::moveDown ()
{
	Tile proposedSet[4];
	ts_copy(tileSet, proposedSet);
	ts_down(proposedSet);
	if ( b->isFree(proposedSet) && b->inBoard(proposedSet) ) 
	{
		ts_copy(proposedSet, tileSet);
		return true;
	}
	else return false;
}

int main(int argc, char *argv[]) {
    Fl_Window *window = new Fl_Window (800,700,"Tetris"); // outer window
	window->color(56); 
	//initializing board
	Board *b = new Board(); 
	//drawing the scoreboard   
   Fl_Box *scorebox = new Fl_Box(tilesize*xmaxtiles+10,50,180,200,"Score: 0");
	scorebox->box(FL_UP_BOX);
        scorebox->labelfont(FL_BOLD+FL_ITALIC);
        scorebox->labelsize(36);
        scorebox->labeltype(FL_ENGRAVED_LABEL);
	b->setScoreBox(&scorebox);	
	window->end();  
   	window->show();
    Fl::add_timeout(timeout, timeractions, b);
   return(Fl::run());  // the process waits from here on for events
}
