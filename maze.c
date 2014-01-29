#include "maze.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <Imlib2.h>


#define MIN(X,Y)	((X < Y) ? (X) : (Y))
#define MAX(X,Y)	((X > Y) ? (X) : (Y))

#define PATH	255
#define WALL	0


enum {
	NORTH=0,
	SOUTH=1,
	EAST=2,
	WEST=3
};

int main(int argc, char **argv){
	img_t *img;
	int count=0;
	srand(time(NULL));
	img = new_img(atoi(argv[1]), atoi(argv[2]));
	gen_maze(img,3,3, &count);
	mk_img_img(img,"output.png");
} 

void gen_maze(img_t *img,int x, int y, int *count){
	int dir;
	int cn=0;
	char name[80];
	img->matrix[x][y] = PATH;
	sprintf(name,"%06d.png",*count);
	mk_img_img(img,name);
	(*count)++;
	//printf("(%d,%d)\n",x,y);
	while(cn<1000){
		dir = rand()%4;
	//	printf("%d\n",dir);

		if(x <= 2 && dir==WEST) continue;
	       	if(y <=	2 && dir==NORTH) continue;
		if(x >= img->width-2 && dir == EAST) continue;
		if(y >= img->height-2 && dir == SOUTH) continue; 

		switch(dir){
			case NORTH:
				if(chk_pt(img,x,y-2)){
					img->matrix[x][y-1] = PATH;
					gen_maze(img,x,y-2,count);
				}
				break;
			case SOUTH:
				if(chk_pt(img,x,y+2)){
					img->matrix[x][y+1] = PATH;
					gen_maze(img,x,y+2,count);
				}
				break;
			case EAST:
				if(chk_pt(img,x+2,y)){
					img->matrix[x+1][y] = PATH;
					gen_maze(img,x+2,y,count);
				}
				break;
			case WEST:
				if(chk_pt(img,x-2,y)){
					img->matrix[x-1][y] = PATH;
					gen_maze(img,x-2,y,count);
				}
				break;

		}
		cn++;
	}
	
}

int new_x(int x, int dir){
	if(dir==EAST) return x+2;
	if(dir==WEST) return x-2;
	return x;
}
int new_y(int y, int dir){
	if(dir==NORTH) return y-2;
	if(dir==SOUTH) return y+2;
	return y;
}

int chk_pt(img_t *img, int x, int y){
	int i;
	int j;
	int a,b;

	a = x-1;
	b = y-1;
	for(i = 0; i< 3; i++){
		for(j = 0; j < 3; j++){
			if(get_val(img,a+i,b+j)==PATH)return 0;	
		}
	}
	return 1;
}
/**
 * This function takes a file name and attempts to load 
 * that file as a img and return a pointer to said img.
 * @param name	This is the name of the file to be loaded.
 * @return 	Returns a pointer to the loaded img.
 */
img_t *ld_img_img(const char *name){
	int w,h;
	int side;
	int i,j;
	Imlib_Color col;

	img_t *newimg;

	Imlib_Image image;

	image = imlib_load_image(name);
	imlib_context_set_image(image);

	w = imlib_image_get_width();
	h = imlib_image_get_height();


	newimg = new_img(w,h);

	for(i=0; i<w; i++){
		for(j=0; j<h; j++){
			imlib_image_query_pixel(i,j,&col);
			newimg->matrix[i][j] = (int) col.red;
		}
	}
	return newimg;


}
/**
 * This function takes in a pointer to a img and outputs the
 * img as a .png. The file name is in the format
 *  <Cost>_<x>_<y>_<h>.png
 *  @param	img	the img to output as an image
 */
void mk_img_img(img_t *img,const char *name){
	int i,j;
	Imlib_Image image;
	int w,h;
	int **mat;
	int val;

	mat = img->matrix;

	w = img->width;
	h = img->height;

	image = imlib_create_image(w,h);

	imlib_context_set_image(image);

	for(i=0; i < w; i++){
		for(j=0; j < h; j++){
			val = mat[i][j];
			imlib_context_set_color(val,val,val,255);
			imlib_image_draw_pixel(i,j,0);	
		}
	}
	
	imlib_save_image(name);
	imlib_free_image();

}

/**
 * wraps a variable to the other side of a img. This way
 * the img loops.
 * @param	x	variable to wrap
 * @param	side	length of a side of the img
 * @return	wrapped variable
 */
inline int wrap(register int x, register int side){
	
	if(x%side==0)return 0;
	if(x < 0){ 
		x = side+(x%side);
	}else{
		x = x % side;
	}
	return x;
	
}
/**
 * gets a value on the img using the wrap function for safety
 * @param	img	img to get from
 * @param	x	x position
 * @param	y	y position
 * @return	value of the img at (x,y)
 */

inline int get_val(img_t *img, int x, int y){
	
	register int w,h;

	
	w = img->width;
	h = img->height;
	
	x = wrap(x,w);
	y = wrap(y,h);

	if(x < 0) return PATH;
	if(x >= w) return PATH;
	if(y < 0) return PATH;
	if(y >= h) return PATH;
	
	return img->matrix[x][y];

}
/**
 * sets a value on a img using the wrap function for safety
 * @param	img	img to set value on
 * @param	x	x position
 * @param	y	y position
 */
inline void set_val(img_t *img, int val, int x, int y){


	register int w,h;

	
	w = img->width;
	h = img->height;

	x = wrap(x,w);
	y = wrap(y,h);

	img->matrix[x][y] = val;
	

}
/**
 * frees a img.
 * @param	img	img to free
 */
void free_img(img_t *img){
	int i;
	int **mat;

	mat = img->matrix;
	
	for(i=0; i < img->width; i++){
		free(mat[i]);
	}
	free(mat);
	free(img);

}


/**
 * callocs a new img.
 * @param	side	length of a side of the img.
 * @return	a pointer to the new img.
 */
img_t *new_img(int w, int h){
	
	//printf("Making new img struct.\n");
	int i;
	img_t *newimg;
	newimg = calloc(1,sizeof(img_t));
	newimg->matrix = calloc(w,sizeof(img_t*));
	for(i=0;i<h;i++){
		newimg->matrix[i] = calloc(h,sizeof(int));
	}
	newimg->width = w;
	newimg->height = h;
	return newimg;

}




