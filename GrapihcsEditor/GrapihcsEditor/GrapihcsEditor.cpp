// GrapihcsEditor.cpp : Defines the entry point for the console application.
//

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<GL\glut.h>
#include "constants.h"

// maximum possible size
float MAXX = 1024, MAXY=700,MAXZ=700;

// used for reshaping
float oldEditorSizeX = MAXX,oldEditorSizeY=MAXY;
int notFirstTime = -1;

// panel constants
float leftMargin = 20, topMargin = MAXY-20, bottomMargin = 10,rightMargin= MAXX-20, optionBoxWidth = 100,optionBoxHeight = 35,space=15;
float leftMargin2 = leftMargin+optionBoxWidth+space;
float colorboxWidth = 20,colorboxHeight = 20;

// constants for defining patterns
int numberOfPatterns = 2, whichPattern=0;

// constants for tracking 
int whichButton=-1, whichColor = 1,btnstate=0 ;

// file pointer to save and open
FILE *fp;

// default brush size
int brushSize = 6;

// for moving drawings
int linex=0,liney=0,elinex=0,eliney=0,ispressed = 0;

// initial spray size
int size = 2;

// initial eraze size
int erazeSize = 2;

//transformation functions
int px,py, drawRectangle =0, reflectionStarted =0,reflecting=0 , translateStarted=0, scaleStarted =0,rotateStarted = 0,scaling=0,translating=0,rotating=0, ax, ay, bx, by;
double c= 0,m=1; // for reflection

// sphere
int sphere =0;

int xoffset = 30,yoffset=15;

// number of options in different panels 
int numTopOptions = 4 ,numLeftOptions=16, numBottomOptions=18;

//float *arr, *clip , *mat;
float arr[1920][1080][3],mat[1920][1080][3],clip[1920][1080][3];
long matsize = MAXX*MAXY*3;

void initialize();
void displayRightBackground();

// returns true if clicked in top area
int inTopArena(int x,int y){
	if(y>topMargin-optionBoxHeight-space)
		return 1;
	return 0;
}

// return true if clicked in bottom area
int inBottomArena(int x,int y){

	if(y<bottomMargin+optionBoxHeight+space)
		return 1;
	return 0;
}

// return true if clicked in editor area
int inEditorArena(int x,int y){

	// xmin,ymin, xmax and ymax of the editor screen 
	int bottom = bottomMargin + colorboxHeight+10+ 2;
	int left = leftMargin2+optionBoxWidth+space + 2;
	int top = topMargin-optionBoxHeight - space - 2;
	int right = rightMargin - 2;

	if (x>= left && x<= right && y<= top && y>= bottom)
		return 1;
	return 0;
}

// return true if clicked in left area
int inLeftArena(int x,int y){
	if(x<= leftMargin2+optionBoxWidth+space)
			return 1;
	return 0;
}

// filled rectangle
void drawRect(int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4){
	glBegin(GL_POLYGON);
		glVertex2i(x1,y1);
		glVertex2i(x2,y2);
		glVertex2i(x3,y3);
		glVertex2i(x4,y4);
	glEnd();
	glFlush();
}

void drawRect(int xmin,int ymin,int xmax,int ymax) {
	drawRect(xmin,ymin,xmax,ymin,xmax,ymax,xmin,ymax);
}

// unfilled rectangle
void drawLineLoop(int xmin,int ymin,int xmax,int ymax){
	glBegin(GL_LINE_LOOP);
	glVertex2i(xmin,ymin);
	glVertex2i(xmax,ymin);
	glVertex2i(xmax,ymax);
	glVertex2i(xmin,ymax);
	glEnd();
	glFlush();
}

// line drawing primitives
void drawLine(int x1,int y1,int x2,int y2){
	glBegin(GL_LINES);
	glVertex2i(x1,y1);
	glVertex2i(x2,y2);
	glEnd();
	glFlush();
}

// erazer
void drawErazer(int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4){
	
	// check for each coord to not go out of editor area
	glBegin(GL_POLYGON);
	
	if(inEditorArena(x1,y1))
		glVertex2i(x1,y1);
	
	if(inEditorArena(x2,y2))
		glVertex2i(x2,y2);
	
	if(inEditorArena(x3,y3))
		glVertex2i(x3,y3);
	
	if(inEditorArena(x4,y4))
		glVertex2i(x4,y4);
	glEnd();
	glFlush();
}

void erazeHandler(int x,int y){
	// set color to white
	glColor3f(1,1,1);
	drawErazer(x,y,x+erazeSize,y,x+erazeSize,y+erazeSize,x,y+erazeSize);
	glFlush();
	glColor3fv(colors[whichColor-1]);
}
// end erazer
 
// filled rectangle
void filledRectHandler(int x,int y) {

	// check for first point clicked
	if(!linex && !liney) {
		glReadPixels(0,0,MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
		linex=x;
		liney=y;
	// second time
	}else {
		elinex=x;
		eliney=y;
		glRasterPos2i(0,0);
		glDrawPixels(MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
		drawRect(linex,liney,elinex,eliney);
	}
}
// filled rectangle done 

// brush
void useBrush(int x,int y){
	glColor3fv(colors[whichColor-1]);
	drawRect(x,y,x+brushSize,y,x+brushSize,y+brushSize,x,y+brushSize);
	glFlush();
}

// end brush

// for pencil
void drawPoint(int x,int y){
		glColor3fv(colors[whichColor-1]);
	if(inEditorArena(x,y)) {
		glBegin(GL_POINTS);
		glVertex2i(x,y);
		glEnd();
		glFlush();
	}
}

// end pencil 

// CIRCLE DRAWING

// draw each point if point in editor area
void draw_pixel(GLint cx,GLint cy)
{
	glColor3fv(colors[whichColor-1]);
	if(inEditorArena(cx,cy)) {
		glBegin(GL_POINTS);
		glVertex2i(cx,cy);
		glEnd();
	}

}

// makes use of circle drawing algo
void plot_pixels(GLint h,GLint k,GLint x,GLint y)
{
	glColor3fv(colors[whichColor-1]);
	draw_pixel(x+h,y+k);
	draw_pixel(-x+h,y+k);
	draw_pixel(x+h,-y+k);
	draw_pixel(-x+h,-y+k);
	draw_pixel(y+h,x+k);
	draw_pixel(-y+h,x+k);
	draw_pixel(y+h,-x+k);
	draw_pixel(-y+h,-x+k);
}


void circle(GLint h,GLint k,GLint r)
{
	GLint d=1-r,x=0,y=r;
	while(y>x)
	{
		plot_pixels(h,k,x,y);
		if(d<0)	d+=2*x+3;
		else
		{
			d+=2*(x-y)+5;
			--y;
		}
		++x;
	}
	plot_pixels(h,k,x,y);
	
}

void initCircle(int x,int y){
	double r=sqrt(pow((double)(elinex-linex),2)+(pow((double)(eliney-liney),2)));
	circle(linex,liney,r);
	glFlush();
	linex=0;
	liney=0;
	elinex=0;
	eliney=0;

}
void circleHandler(int x,int y){
	if(!linex && !liney) {
		glReadPixels(0,0,MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
		linex=x;
		liney=y;
	}else{
		elinex=x;
		eliney=y;
		glRasterPos2i(0,0);
		glDrawPixels(MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
		double r=sqrt(pow((double)(elinex-linex),2)+(pow((double)(eliney-liney),2)));
		circle(linex,liney,r);				
		glFlush();			
	}
}


// END OF CIRCLE DRAWING

// line handler
void lineHandler(int x,int y){
	if(!linex && !liney) {
		glReadPixels(0,0,MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
		linex=x;
		liney=y;
	}else {
		elinex=x;
		eliney=y;
		glRasterPos2i(0,0);
		glDrawPixels(MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
		drawLine(linex,liney,x,y);
		glFlush();
	}
}
// end of line


// spray
void spray(int x,int y)
{
	int randx,randy;
	for(int i=0;i<50;i++)
	{
		randx=x + rand()%(size/2);
		randy=y + rand()%(size/2);
		drawPoint(randx,randy);
	}
	glFlush();
}

void useSpray(int x,int y){
	glPointSize(1.0);
	spray(x,y);
	glFlush();

}
// end spray
void draw_text(char *info,int i,int j)
{
//	printf("%s\n",info);
	glRasterPos2i(i+xoffset,j+yoffset);
	while(*info)
	{
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10,*info++);
	}
	glFlush();
}

// draw Rectangle
void rectHandler(int x,int y){
	if(!linex && !liney) {
		glReadPixels(0,0,MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
		linex=x;
		liney=y;
	}else{
		elinex=x;
		eliney=y;
		glRasterPos2i(0,0);
		glDrawPixels(MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
		drawLineLoop(linex,liney,elinex,eliney);
	}
}

// end pattern

// cube Handler not fully implemented
void drawCube(int x1,int y1,int x2,int y2) {
	int sidelength = x2-x1;
	if(sidelength<0)
		sidelength = sidelength*-1;
	
	int z1=0,z2;
	x2 = x1+sidelength, y2 = y1+sidelength, z2 = z1+sidelength;
	
	glColor3fv(colors[whichColor-1]);

	//xy
	glBegin(GL_LINE_LOOP);
	glVertex3f(x1,y1 ,z1 );
	glVertex3f(x2,y1 ,z1 );
	glVertex3f(x2,y2 ,z1 );
	glVertex3f(x1,y2 ,z1 );
	glEnd();

	glBegin(GL_LINE_LOOP);

	//face in yz plane
	glVertex3f(x1,y2 ,z1 );
	glVertex3f(x2,y2 ,z1 );
	glVertex3f(x2,y2 ,z2 );
	glVertex3f(x1,y2 ,z2 );
	glEnd();

	//face in zx plance
	glBegin(GL_LINE_LOOP);
	glVertex3f(x1,y1 ,z1 );
	glVertex3f(x2,y1 ,z1 );
	glVertex3f(x2,y1 ,z2 );
	glVertex3f(x1,y1 ,z2 );
	glEnd();

	//|| to yz plane.
	glBegin(GL_LINE_LOOP);
	glVertex3f(x2,y2 ,z1 );
	glVertex3f(x1,y2 ,z1 );
	glVertex3f(x2,y1 ,z2 );
	glVertex3f(x2,y2 ,z2 );
	glEnd();

	//face in yz plane
	glBegin(GL_LINE_LOOP);
	glVertex3f(x1,y1 ,z1 );
	glVertex3f(x1,y1 ,z2 );
	glVertex3f(x1,y2 ,z2 );
	glVertex3f(x1,y2 ,z1 );
	glEnd();


	//|| to xy plane
	glBegin(GL_LINE_LOOP);
	glVertex3f(x1,y2 ,z2 );
	glVertex3f(x1,y1 ,z2 );
	glVertex3f(x2,y1 ,z2 );
	glVertex3f(x2,y2 ,z2 );
	glEnd();

	glFlush();

}
void cubeHandler(int x,int y){
	if(!linex && !liney) {
		glReadPixels(0,0,MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
		linex = x;
		liney = y;
	}else {
		elinex = x;
		eliney = y;
		glRasterPos2i(0,0);
		glDrawPixels(MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
		drawCube(linex,liney,elinex,eliney);
		glFlush();
	}
}
// end cube


// sphere

void drawSphere(int h,int k,int r){
	float theta = 0, phi = 0, deltaTheta = PI / 30.0, deltaPhi = PI /30.0;
	float z1, x1, y1, z2, x2, y2, z3, x3, y3, z4, x4, y4;	
	
	glBegin(GL_LINE_LOOP);

	for(theta = 0; theta <= 2 * PI ; theta += deltaTheta)
	{
		for(phi = 0; phi <= PI; phi += deltaPhi)
		{

			z1 = r * sinf(phi + deltaPhi) * cosf(theta + deltaTheta);
			x1 = r * sinf(phi + deltaPhi) * sinf(theta + deltaTheta);
			y1 = r * cosf(phi + deltaPhi);

			z2 = r * sinf(phi) * cosf(theta + deltaTheta);
			x2 = r * sinf(phi) * sinf(theta + deltaTheta);
			y2 = r * cosf(phi);

			z3 = r * sinf(phi) * cosf(theta);
			x3 = r * sinf(phi) * sinf(theta);
			y3 = r * cosf(phi);

			z4 = r * sinf(phi + deltaPhi) * cosf(theta);
			x4 = r * sinf(phi + deltaPhi) * sinf(theta);
			y4 = r * cosf(phi + deltaPhi);

			
			glColor3fv(colors[whichColor-1]);
			//if(inEditorArena(h+x2,h+y2))
			glVertex3f(h+x2,k+y2, z2);
			
		//	if(inEditorArena(h+x3,h+y3))
			//glColor3f(0,0,1);
			glVertex3f(h+x3,k+y3, z3);	

		//	if(inEditorArena(h+x1,h+y1))	
			//glColor3f(1,1,1);		
			glVertex3f(h+x1,k+y1, z1);

		//	if(inEditorArena(h+x4,h+y4))
			//glColor3f(0.176,0.224,0.230);	
			glVertex3f(h+x4,k+y4, z4);

			
		}
	}
	glEnd();
	return;
}

void sphereHandler(int x,int y) {
	
	int bottom = bottomMargin + colorboxHeight+10;
	int left = leftMargin2+optionBoxWidth+space;
	int top = topMargin-optionBoxHeight - space;
	int right = rightMargin;

	if(!linex && !liney) {
		glReadPixels(0,0,MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
		linex=x;
		liney=y;
	}	else{
		sphere=1;
		elinex=x;
		eliney=y;
		glRasterPos2i(0,0);
		glDrawPixels(MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
		double r=sqrt(pow((double)(elinex-linex),2)+(pow((double)(eliney-liney),2)));
		
		if(linex-r<left)
			r = linex-left;
		if(liney-r<bottom)
			r = liney - bottom;
		if((liney+r)>top)
			r = top - liney;
		if(linex+r > right)
			r = right - linex;


		drawSphere(linex,liney,r);				
		glFlush();			
	}
}
// end sphere

// inside clip Handler
void insideClipHandler(int x,int y){
	glPointSize(3);
	if(!linex && !liney) {
		glReadPixels(0,0,MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
		linex=x;
		liney=y;
	}else{
		elinex=x;
		eliney=y;
		glRasterPos2i(0,0);
		glDrawPixels(MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
		glColor3f(1,1,1);
		drawLineLoop(linex,liney,elinex,eliney);
		glColor3fv(colors[whichColor-1]);
	}
	glPointSize(1);
}

// outside clip handler
void outsideClipHandler(int x,int y){
	glPointSize(3);
	if(!linex && !liney) {
		glReadPixels(0,0,MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
		linex=x;
		liney=y;
	}else{
		elinex=x;
		eliney=y;
		glRasterPos2i(0,0);
		glDrawPixels(MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
		glColor3f(1,1,1);
		drawLineLoop(linex,liney,elinex,eliney);
		glColor3fv(colors[whichColor-1]);
	}						
	glPointSize(1);
}
// handle translate
void translateHandler(int x,int y) {
	if(!linex && !liney){
		glReadPixels(0,0,MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
		linex=x;
		liney=y;
	}else{
		elinex=x;
		eliney=y;
		glRasterPos2i(0,0);
		glDrawPixels(MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
		drawLineLoop(linex,liney,elinex,eliney);
		glColor3fv(colors[0]);
	}				
	drawRectangle = 1;
	translateStarted = 1;
	translating =1;
}

void translate(int x,int y) {
	glRasterPos2i(0,0);
	glDrawPixels(MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
	if (inEditorArena(x,y)){
		glRasterPos2i(x,y);
		glDrawPixels(px-1,py-1,GL_RGB,GL_FLOAT,clip);		
	}
	glFlush();
}

// end translator

//handle scale
void scaleHandler(int x,int y) {
	if(!linex && !liney) {
		glReadPixels(0,0,MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
		linex=x;
		liney=y;
	}else{
		scaleStarted = 1;
		scaling = 1;
		drawRectangle=1;
		elinex=x;
		eliney=y;
		glRasterPos2i(0,0);
		glDrawPixels(MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
		drawLineLoop(linex,liney,elinex,eliney);
	}
}

void scale(int x,int y){
	glRasterPos2i(0,0);
	glDrawPixels(MAXX,MAXY,GL_RGB,GL_FLOAT,arr);

	if(inEditorArena(x,y)) {
		glPixelZoom(2,2);
		glRasterPos2i(x,y);
		glDrawPixels(px-1,py-1,GL_RGB,GL_FLOAT,clip);		
	}
	glFlush();
	displayRightBackground();
	glPixelZoom(1,1);
}
//end scale

// handle rotate
void rotateHandler(int x,int y){
	if(!linex && !liney) {
		glReadPixels(0,0,MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
		linex=x;
		liney=y;
	}else{
		rotateStarted = 1;
		drawRectangle = 1;
		rotating =1;
		elinex=x;
		eliney=y;
		glRasterPos2i(0,0);
		glDrawPixels(MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
		drawLineLoop(linex,liney,elinex,eliney);
	}
}

void rotate(int x,int y){
	glColor3fv(colors[whichColor-1]);
	glRasterPos2i(0,0);
	glDrawPixels(MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
	if(inEditorArena(x,y)){
		glPushMatrix();
		glTranslatef(x,y,0);
		glRotatef(30,0,0,1);
		glTranslatef(-x,-y,0);
		drawLineLoop(x,y,x+px,y+py);
		glFlush();
		glPopMatrix();
	}
	glFlush();
	rotateStarted = 0;
}

//end rotate

// handle reflection

void reflect(int x,int y){
	int bottom = bottomMargin + colorboxHeight+10;
	int left = leftMargin2+optionBoxWidth+space;
	int top = topMargin-optionBoxHeight - space;
	int right = rightMargin;
	glColor3fv(colors[whichColor-1]);
	glRasterPos2i(0,0);
	glDrawPixels(MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
	if(inEditorArena(x,y)){
		glPushMatrix();
		GLfloat theta = atan(m)*180.0/3.14159;
		glTranslatef(left,bottom,0);
		glTranslatef(0,c,0);
		glRotatef(theta,0,0,1);
		glScalef(1,-1,0);
		glRotatef(-theta,0,0,1);
		glTranslatef(0,-c,0);
		glTranslatef(-left,-bottom,0);
		drawLineLoop(ax,ay,bx,by);
		glFlush();
		glPopMatrix();
	}
	glFlush();
	reflectionStarted=0;	
}
void reflectHandler(int x,int y) {
	if(!linex && !liney) {
		glReadPixels(0,0,MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
		linex=x;
		liney=y;
	}else{
		reflectionStarted = 1;
		reflecting = 1;
		drawRectangle = 1;
		elinex=x;
		eliney=y;
		glRasterPos2i(0,0);
		glDrawPixels(MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
		drawLineLoop(linex,liney,elinex,eliney);
	}

}
// end reflection

// pattern helpers
void patternHandler(int x,int y){
	if(!linex && !liney) {
		glReadPixels(0,0,MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
		linex=x;
		liney=y;
	}else{
		drawRectangle=1;
        elinex=x;
		eliney=y;
		glRasterPos2i(0,0);
		glColor3fv(colors[whichColor-1]);
		glDrawPixels(MAXX,MAXY,GL_RGB,GL_FLOAT,arr);
		// pattern fill
        glEnable(GL_POLYGON_STIPPLE);          	
		glPolygonStipple(halftone[whichPattern]);              	
		drawRect(linex,liney,elinex,eliney);
		glDisable(GL_POLYGON_STIPPLE);  
     	glFlush();
	}
}

// display sample patters at bottom
void displayPatterns(){
	int i;
	int leftCoord = leftMargin+space+(numBottomOptions+1)*(colorboxWidth)+2*space;
	int bottom = bottomMargin;
	int topCord = bottom+colorboxHeight;

	for(i=0;i<numberOfPatterns;i++){

		int left = leftCoord+i*(colorboxWidth+1);
		int right = left+colorboxWidth;

		glColor3fv(colors[7]);
		glEnable(GL_POLYGON_STIPPLE);          	
		glPolygonStipple(halftone[i]);              	
		drawRect(left,bottom,right,topCord);
		glDisable(GL_POLYGON_STIPPLE);  
		
		glColor3fv(colors[0]);
		drawLineLoop(left,bottom,right,topCord);
		
		glFlush();
	}
	glColor3fv(colors[whichColor-1]);

}
// determint which pattern was chosen
int oneOfPatterns(int x,int y){
	int i;
	int leftCoord = leftMargin+space+(numBottomOptions+1)*(colorboxWidth)+2*space;
	int bottom = bottomMargin;
	int topCord = bottom+colorboxHeight;
	for(i=0;i<numberOfPatterns;i++){
		int left = leftCoord+i*(colorboxWidth+1);
		int right = left+colorboxWidth;
		if(x>=left && x<= right && y>=bottom && y<= topCord) {
			glColor3f(1,0,1);
			glPointSize(2);
			drawLineLoop(left,bottom,right,topCord);
			whichPattern = i;
			glPointSize(1);
			break;
		}
	}
	// recolor outer boundary of not selected patterns to white
	for(i=0;i<numberOfPatterns;i++){
		int left = leftCoord+i*(colorboxWidth+1);
		int right = left+colorboxWidth;
		if(whichPattern!=i) {
			glColor3fv(colors[7]);
			drawLineLoop(left,bottom,right,topCord);
		}
	}
	glColor3fv(colors[whichColor-1]);
	return whichPattern;
}

// when another options is chose undisplay the pattern options
void undisplayPatterns(){
	int leftCoord = leftMargin+space+(numBottomOptions+1)*(colorboxWidth)+2*space;
	int bottom = bottomMargin;
	int topCord = bottom+colorboxHeight;
	int rightCoord = leftCoord+(numberOfPatterns)*(colorboxWidth+1);
	
	glColor3fv(colors[17]);
	drawRect(leftCoord-1,bottom,rightCoord,topCord);
	glColor3fv(colors[whichColor-1]);
	glFlush();
}

// end of pattern helpers

// return which color was chosen
int oneOfBottom(int x,int y){

	for (int boxNumber=1;boxNumber<= numBottomOptions;boxNumber++){
		int bottomCord = bottomMargin;
		int leftCord = leftMargin+space+(boxNumber)*(colorboxWidth+1);
		int topCoord = bottomCord+colorboxHeight;
		int rightCoord = leftCord+colorboxWidth;

		if( x >= leftCord && x<= rightCoord && y>= bottomCord && y<= topCoord)
			return boxNumber;
	}
	return whichColor;
}
// return which option from top was chosen
int oneOfTop(int x,int y){
	for (int boxNumber=1 ;boxNumber<=numTopOptions; boxNumber++){

		int topCoord = topMargin;
		int bottomCord = topMargin-optionBoxHeight;
		int leftCord = leftMargin+(boxNumber-1)*(optionBoxWidth+space);
		int rightCoord = leftCord+optionBoxWidth;
		
		if( x >= leftCord && x<= rightCoord && y>= bottomCord && y<= topCoord)
			return boxNumber;
	}
	return -1;
}
// return true if pressed button one of left box
int inLeftBox(int boxNumber,int leftMarg,int count,int x,int y){
	int leftBoxTop = topMargin- 2*optionBoxHeight - space;
	int bottomCord = leftBoxTop - (optionBoxHeight+space)*(count-1);
	int leftCord = leftMarg;
	int rightCoord = leftMarg+optionBoxWidth;
	int topCoord = bottomCord+optionBoxHeight;

	if( x >= leftCord && x<= rightCoord && y>= bottomCord && y<= topCoord)
			return 1;
	return 0;
	
}

// return which option from the left

int oneOfLeft(int x,int y){
	int boxNumber = 0;
	int count=1;
	for (int i=0;i<11;i++){
		if (inLeftBox(i+1,leftMargin,count++,x,y) )
			return i+1+numTopOptions;
	}
	count = 1 ;
	for (int i=11;i<numLeftOptions;i++){
		if (inLeftBox(i+1,leftMargin2,count++,x,y))
			return i+1+numTopOptions;
	}
	return whichButton;
}

// draws the left boxes i.e options to left of editor
void leftBox(int boxNumber,int leftMarg,int count,GLfloat bcolor[],GLfloat tcolor[]){
	int leftBoxTop = topMargin- 2*optionBoxHeight - space;
	int top = leftBoxTop - (optionBoxHeight+space)*(count-1);
	glColor3fv(bcolor);
	drawRect(leftMarg ,top,  leftMarg+optionBoxWidth,top,leftMarg+optionBoxWidth ,top+optionBoxHeight,leftMarg , top+optionBoxHeight );
	glColor3fv(tcolor);
	draw_text(leftBoxOptions[boxNumber-1],leftMarg ,top);
}

// draws topoptions
void topBox(int boxNumber,GLfloat bcolor[],GLfloat tcolor[]){

	int top = topMargin-optionBoxHeight;
	int leftCord = leftMargin+(boxNumber-1)*(optionBoxWidth+space);

	glColor3fv(bcolor);
	drawRect(leftCord,top , leftCord + optionBoxWidth, top, leftCord + optionBoxWidth,topMargin,leftCord,topMargin);

	glColor3fv(tcolor);
	draw_text(topOptions[boxNumber-1],leftCord,top);
}

// displays the background to right of the editor
// used in case of scaling and reshaping to redraw right part on the overwrited editor

void displayRightBackground(){
	glColor3fv(colors[17]);
	drawRect(rightMargin,0,MAXX,MAXY);
	glFlush();
	glColor3fv(colors[whichColor-1]);
}

// draw the box which shows the current color chosen
void drawBottomBigBox(float color[3]){
	int bottom = bottomMargin;
	int leftCord = leftMargin;
	int height = colorboxHeight+10;
	int width = colorboxWidth+10;
	glColor3fv(color);
	drawRect(leftCord,bottom , leftCord + width, bottom, leftCord + width,bottom+height,leftCord,bottom+height);
}

// draw boxes at bottom i.e color boxes
void bottomBox(int boxNumber){
	int bottom = bottomMargin;
	int leftCord = leftMargin+space+(boxNumber)*(colorboxWidth+1);
	glColor3fv(colors[boxNumber-1]);
	drawRect(leftCord,bottom , leftCord + colorboxWidth, bottom, leftCord + colorboxWidth,bottom+colorboxHeight,leftCord,bottom+colorboxHeight);
}

// displays the options on top
void displayTopOptions() {
	int bottomOfTopPanel = topMargin- optionBoxHeight - space;
	glColor3fv(colors[17]);
	drawRect(0,bottomOfTopPanel,MAXX,MAXY);
	for (int i=0;i<numTopOptions;i++){
		topBox(i+1,colors[0],colors[7]);
	}
}
//like wise
void displayBottomOptions() {

	float black[] = {0,0,0};
	drawBottomBigBox(black);
	for (int i=0;i<numBottomOptions;i++){
		bottomBox(i+1);
	}
}
//yeah again
void displayLeftOptions(){
	int count=1;
	for (int i=0;i<11;i++){
		leftBox(i+1,leftMargin,count++,colors[0],colors[7]);
	}
	count = 1 ;
	for (int i=11;i<numLeftOptions;i++){
		leftBox(i+1,leftMargin2,count++,colors[0],colors[7]);
	}
}
// display the editor for user to scratch 
void displayEditorScreen(){
	int bottom = bottomMargin + colorboxHeight+10;
	int left = leftMargin2+optionBoxWidth+space;
	int top = topMargin-optionBoxHeight - space;
	int right = rightMargin;
	glColor3f(1,1,1);
	drawRect(left,bottom,right,bottom,right,top,left,top);
}

// open and load the contents of saved file 
void openHandler(){
	int bottom = bottomMargin + colorboxHeight+10;
	int left = leftMargin2+optionBoxWidth+space;
	int top = topMargin-optionBoxHeight - space;
	int right = rightMargin;

	char filename[30];
	printf("\nEnter filename to open : ");
	scanf("%s",filename);
	fp=fopen(filename,"r");
	while(fp==NULL){
		printf("\nFile \"%s\" does not exist. Enter an existing file Name: ",filename);
		scanf("%s",filename);
		fp=fopen(filename,"rb");
	}
	// read from file to matrix
	fread(mat,matsize,sizeof(float),fp);
	fclose(fp);			
	displayEditorScreen();
	// write on screen
	glRasterPos2i(left,bottom);
	glDrawPixels(right-left,top-bottom,GL_RGB,GL_FLOAT,mat);
	printf("\nFILE OPENED\n");
	glFlush();
}
//end open

// save handler
void saveHandler(){

	int bottom = bottomMargin + colorboxHeight+10;
	int left = leftMargin2+optionBoxWidth+space;
	int top = topMargin-optionBoxHeight - space;
	int right = rightMargin;

	char filename[30];
	glReadPixels(left,bottom,right-left,top-bottom,GL_RGB,GL_FLOAT,mat);
	printf("Enter the filename to save : ");
	scanf("%s",filename);
	fp=fopen(filename,"wb");			
	fwrite(mat,matsize,sizeof(float),fp);
	fclose(fp);
	printf("\nFILE SAVED\n");	
	glFlush();
}
// end save

// provide help statements
void provideHelp(){
	int bottom = 0;
	int left = numBottomOptions*colorboxWidth + space + 3*space;
	
	glColor3fv(colors[17]);
	drawRect(left,bottom,MAXX,bottomMargin+colorboxHeight+10);
	
	if(whichButton==PATTERNFILL){
		left += (numberOfPatterns*colorboxWidth)+2*space;
	}
	glColor3fv(colors[7]);
	draw_text(helpBox[whichButton-1],left,bottom);
	glColor3fv(colors[whichColor-1]);
}

// to unselect the previous chosen button once a new button is clicked
void undoPrevButton(int prevbtn){
	if(btnstate) {
		if(prevbtn==PATTERNFILL)
			undisplayPatterns();
//		provideHelp();
		if(prevbtn<=0) return;
	//	printf("prevbutton %d current button: %d",prevbtn,whichButton);
		if (prevbtn <= 4)
			topBox(prevbtn,colors[0],colors[7]);
		else if(prevbtn<=(11+numTopOptions) && prevbtn > numTopOptions)
			leftBox(prevbtn-numTopOptions,leftMargin,prevbtn-numTopOptions,colors[0],colors[7]);
		else 
			leftBox(prevbtn-numTopOptions,leftMargin2,prevbtn-(11+numTopOptions),colors[0],colors[7]);
		}
}
void highLightButton(){
	if(whichButton==-1) return;

	if (whichButton <= 4)
		topBox(whichButton,colors[7],colors[0]);
		else if(whichButton<=(11+numTopOptions) && whichButton > numTopOptions)
			leftBox(whichButton-numTopOptions,leftMargin,whichButton-numTopOptions,colors[7],colors[0]);
		else 
			leftBox(whichButton-numTopOptions,leftMargin2,whichButton-(11+numTopOptions),colors[7],colors[0]);
	if(whichButton == PATTERNFILL)
		displayPatterns();
	provideHelp();
}

void hightLightHelper(int prevbtn){
	if(whichButton != prevbtn && whichButton!=-1 ){
		btnstate = 1;
		undoPrevButton(prevbtn);
		highLightButton();
	}
}
// the mouse callback function:
void onMouseMove(int x, int y)
{
	y = MAXY-y;
	if (inEditorArena(x,y) &&  ispressed){
		glColor3fv(colors[whichColor-1]);
		if(btnstate){
			switch(whichButton) {
				case PENCIL:
					glPointSize(2);
					drawPoint(x,y);
				break;
				case BRUSH:
					useBrush(x,y);
					glFlush();
				break;
				case SPRAY:
					useSpray(x,y);
					break;
				case ERAZER:
					erazeHandler(x,y);
					break;
				case RECTANGLE:
					rectHandler(x,y);
					break;
				case FILLEDRECT:
					filledRectHandler(x,y);
					break;
				case CIRCLE:
					circleHandler(x,y);
					break;
				case LINE:
					lineHandler(x,y);
					break;
				case INSIDECLIP:
					insideClipHandler(x,y);
					break;
				case OUTSIDECLIP:
					outsideClipHandler(x,y);
					break;
				case SPHERE:
					sphereHandler(x,y);
					break;
				case PATTERNFILL:
					patternHandler(x,y);
					break;
				case TRANSLATE:
					translateHandler(x,y);
					break;
				case SCALE:
					scaleHandler(x,y);
					break;
				case ROTATE:
					rotateHandler(x,y);
					break;
				case REFLECT:
					reflectHandler(x,y);
					break;
				case CUBE:
					cubeHandler(x,y);
					break;
				}		
			}
	}
 
}
void onMouseClick(int button,int state,int x,int y){
	y = MAXY-y;
	int prevbtn;
	
	int bottom = bottomMargin + colorboxHeight+10;
	int left = leftMargin2+optionBoxWidth+space;
	int top = topMargin-optionBoxHeight - space;
	int right = rightMargin;
	glColor3fv(colors[whichColor-1]);
	if(state==GLUT_DOWN && button==GLUT_LEFT_BUTTON){
		prevbtn = whichButton;
		ispressed = 1;
		if(inTopArena(x,y) ){
			whichButton = oneOfTop(x,y);
		//	printf("prevbutton %d current button: %d",prevbtn,whichButton);
			hightLightHelper(prevbtn);
			// handle top options
			switch(whichButton) {
			case NEW:
					displayEditorScreen();
					return;
				case OPEN:
					openHandler();
					return;
				case SAVE:
					saveHandler();
					return;
				case EXIT:
					exit(0);
			}
		}else if  (inBottomArena(x,y)){
			if(whichButton==PATTERNFILL) {
				whichPattern = oneOfPatterns(x,y);
			}
			whichColor = oneOfBottom(x,y);
			drawBottomBigBox(colors[whichColor-1]);
			glColor3fv(colors[whichColor-1]);

		}else if (inLeftArena(x,y)){
			whichButton = oneOfLeft(x,y);
			hightLightHelper(prevbtn);

			switch(whichButton){
				case PATTERNFILL:
					displayPatterns();
					break;
			}
		}else if (inEditorArena(x,y)){
			glColor3fv(colors[whichColor]);
			if(btnstate){
				switch(whichButton){
				case PENCIL:
					glPointSize(2);
					drawPoint(x,y);
					break;
				case BRUSH:
					useBrush(x,y);
					break;
				case SPRAY:
					useSpray(x,y);
					break;
				case ERAZER:
					erazeHandler(x,y);
					break;
				case RECTANGLE:
					rectHandler(x,y);
					break;
				case FILLEDRECT:
					filledRectHandler(x,y);
					break;
				case CIRCLE:
					initCircle(x,y);
					break;
				case LINE:
					lineHandler(x,y);
					break;
				case INSIDECLIP:
					insideClipHandler(x,y);
					break;
				case OUTSIDECLIP:
					outsideClipHandler(x,y);
					break;
				case CUBE:
					cubeHandler(x,y);
					break;
				}
			}
		}
		if(scaleStarted){
			scale(x,y);
			scaleStarted = 0;
		}
	
		if(rotateStarted) {
			rotate(x,y);
			rotateStarted =0;
		}

		if(translateStarted) {
			translate(x,y);
			translateStarted =0;
		}

		if(reflectionStarted) {
			reflect(x,y);
			reflectionStarted = 0;
		}
	}
	

	if(state==GLUT_UP) {
		if(inEditorArena(x,y)) {
			switch (whichButton) {
			case INSIDECLIP:
				glColor3f(1,1,1);
				if(!linex || !liney || !elinex || !eliney)
					break;
				drawRect(linex,liney,elinex,eliney);
				glColor3fv(colors[whichColor-1]);
				glFlush();
				break;
			case OUTSIDECLIP:
				px=elinex-linex; 
				py=liney-eliney;
				if(!linex || !liney || !elinex || !eliney)
					break;
				glReadPixels(linex,eliney,px,py,GL_RGB,GL_FLOAT,clip);
				displayEditorScreen();
				glFlush();
				glRasterPos2i(linex,eliney);
				glDrawPixels(px,py,GL_RGB,GL_FLOAT,clip);
				glFlush();
				break;
			}
			if(scaling) {
				ax=linex;ay=liney;bx=elinex;by=eliney;
				px=elinex-linex; py=liney-eliney;
				glReadPixels(linex,eliney+1,px-1,py-1,GL_RGB,GL_FLOAT,clip);			
				scaleStarted=1;
				scaling =0;
			}
			if(translating){
				ax=linex, ay=liney, bx=elinex, by=eliney;
				px = elinex-linex;
				py=liney-eliney;
				glReadPixels(linex,eliney+1,px-1,py-1,GL_RGB,GL_FLOAT,clip);
				translateStarted =1;
				translating =0;
			}
			if (rotating) {
				ax=linex;ay=liney;bx=elinex;by=eliney;
				px=elinex-linex; py=liney-eliney;
				rotateStarted = 1;
				glFlush();
				rotating = 0;
			}
			if(reflecting){
				ax=linex;ay=liney;bx=elinex;by=eliney;
				px=elinex-linex; py=liney-eliney;
				reflectionStarted = 1;
				glFlush();
				reflecting = 0;
			}
			if(drawRectangle){
				drawLineLoop(linex,liney,elinex,eliney);
				drawRectangle =0;
			}
			if(sphere) {
				
				double r=sqrt(pow((double)(elinex-linex),2)+(pow((double)(eliney-liney),2)));
				
				if(linex-r<left)
					r = linex-left;
				if(liney-r<bottom)
					r = liney - bottom;
				if((liney+r)>top)
					r = top - liney;
				if(linex+r > right)
					r = right - linex;

				drawSphere(linex,liney,r);
				glFlush();
				sphere=0;
			}
			// undo all the initialzations done while drawing moving objects 
			ispressed = 0;
			linex=0;
			liney=0;
			elinex=0;
			eliney=0;
		}
		// after every click, read the state of the editor window into the matrix
		glReadPixels(left,bottom,right-left,top-bottom,GL_RGB,GL_FLOAT,mat);
		// old contents of size oldeditorsize is used later to display in the new resized editor
		oldEditorSizeX = right-left;
		oldEditorSizeY = top-bottom;
	}
}

// MAIN display callback of the application
void displayscreen(){
	int bottom = bottomMargin + colorboxHeight+10;
	int left = leftMargin2+optionBoxWidth+space;
	int top = topMargin-optionBoxHeight - space;
	int right = rightMargin;
	
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(0,0,0);

	// check whether the window display was called the first time, 
	if(notFirstTime>0)
	{
		// display the new resized editor screen
		displayEditorScreen();
		glRasterPos2i(left,bottom);
		//retain the contents
		glDrawPixels(oldEditorSizeX,oldEditorSizeY,GL_RGB,GL_FLOAT,mat);
		//printf("state written back\n");
		
		//display the panels
		displayTopOptions();	
		displayLeftOptions();
		displayBottomOptions();
		displayRightBackground();
		highLightButton();
	}
	else //  if it is then normal initializations and display
	{
		displayTopOptions();	
		displayLeftOptions();
		displayBottomOptions();
		displayEditorScreen();
		// incase the window is resized wihtout doing anything, it crashes... so read the state into matrix once 
		glReadPixels(left,bottom,right-left,top-bottom,GL_RGB,GL_FLOAT,mat);
		oldEditorSizeX = right-left;
		oldEditorSizeY = top-bottom;

	}
	glFlush();
}

// keyboard click handler
void keyboard(unsigned char keyPressed, int x, int y)
{	
	// to resize erazer dynamically
	if (whichButton== ERAZER) {
		if(keyPressed == '+') {
			erazeSize++;
		}else if(keyPressed == '-'){
			if(size>2)
				erazeSize--;
		}
	}
	// to resize sprayer dynamically
	else if (whichButton==SPRAY){
		if(keyPressed == '+') {
			size++;
		}else if(keyPressed == '-'){
			if(size>2)
				size--;
		}
	}
}

// intializing the coordinates and projection matrix
void initialize()
{		
	glClearColor(0.176,0.224,0.230,0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0,MAXX,0.0,MAXY,0.0,MAXZ);
	glMatrixMode(GL_MODELVIEW);
	glColor3fv(colors[0]);
}

// handles the reshaping of the windows
void handleReshape(int newWidth,int newHeight)
{
	
	// scaling factors 
	float scaleX = newWidth/MAXX;
	float scaleY = newHeight/MAXY;
	float *temp;
	
	// rescale all the constant coordinates according to the new window size, rescale X coords by scaleX factor, Ycoords by scaleY factor
	MAXX = MAXX*scaleX , MAXY=MAXY*scaleY ,MAXZ=700;
	float oldsize = matsize;
	matsize = MAXX*MAXY*3;

	leftMargin = leftMargin*scaleX ,topMargin =topMargin * scaleY , bottomMargin = bottomMargin*scaleY ,rightMargin= rightMargin*scaleX , optionBoxWidth = optionBoxWidth*scaleX, optionBoxHeight = optionBoxHeight * scaleY;
	space=space*scaleX;
	leftMargin2 = leftMargin2*scaleX;
	colorboxWidth = colorboxWidth*scaleX ,colorboxHeight =colorboxHeight*scaleY;

	// to track the reshaping
	notFirstTime++;
	// set viewport to new size of the window
	glViewport(0,0,newWidth,newHeight);
	// reinitialize
	initialize();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(MAXX, MAXY);
	glutInitWindowPosition(0,0);
	glutCreateWindow("Graphical Editor");	
	initialize();
	glutDisplayFunc(displayscreen);
	glutMouseFunc(onMouseClick);
	glutMotionFunc(onMouseMove);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(handleReshape);
	glutMainLoop();
}
