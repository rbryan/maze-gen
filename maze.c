#include "maze.h"
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <Imlib2.h>
#include <sys/resource.h>


#define MIN(X,Y)	((X < Y) ? (X) : (Y))
#define MAX(X,Y)	((X > Y) ? (X) : (Y))

#define NORTH	1
#define SOUTH	2
#define EAST	4
#define WEST	8

#define PATH	255
#define WALL	0

#define N1	0
#define N2	1
#define N3	2
#define N4	3
#define CX	4
#define CY	5
#define ND	6
#define START	7
#define END	8
#define USED	9

#define NODE_SIZE	10



int main(int argc, char **argv){
	img_t *img;
//	int count=0;
	if(argc < 2){
		fprintf(stderr, "Usage: maze <mode> {s: <img> } {g: <m> <n> <output> <stack size (MB)>}\n");
		exit(1);
	}
	if(*argv[1] == 'g'){
		if(argc < 6){
			fprintf(stderr, "Usage: maze g <mode> <m> <n> <output> <stack size (MB)>\n");
		}
		set_stack_size(atoi(argv[5]));	
	
		srand(time(NULL));
		img = new_img(2*atoi(argv[2])+1, 2*atoi(argv[3])+1);
		gen_maze(img,3,3, NULL);
		mk_img_img(img,argv[4]);
		return 0;
	}else if(*argv[1] == 's'){
		img_t *img;
		graph_t *graph;


		img = ld_img_img("test.png");
		mk_img_img(img,"output.png");
		graph = get_graph(img);
		print_graph(graph);

	}else{
		fprintf(stderr,"Usage: maze <mode> {s: <img> } {g: <m> <n> <output> <stack size (MB)}");
		exit(1);
	}

	return 0;
}

void set_stack_size(int mb){
	const rlim_t kStackSize = mb * 1024 * 1024;   // increase the minimum stack size. 
	struct rlimit rl;
	int result;
	result = getrlimit(RLIMIT_STACK, &rl);
	if (result == 0)
	{
	        if (rl.rlim_cur < kStackSize)
	        {
	                rl.rlim_cur = kStackSize;
		        result = setrlimit(RLIMIT_STACK, &rl);
		        if (result != 0)
		        {
		                   fprintf(stderr, "setrlimit returned result = %d\n", result);
		        }
		}
	}
}

graph_t *get_graph(img_t *img){
	int w,h;
	int sx,sy;
	graph_t *newgraph;

	w = img->width;
	h = img->height;
	newgraph = new_graph(img);

	sx = newgraph->sx;
	sy = newgraph->sy;
	
		printf("%d %d\n",sx,sy);
		fflush(stdout);
	if(sx==0){
		printf("%d %d\n",sx,sy);
		fflush(stdout);
		gg_r(img,newgraph,sx,sy,0,WEST);
	}else if(sy==0){
		gg_r(img,newgraph,sx,sy,0,SOUTH);
	}else if(sx==(w-1)){
		gg_r(img,newgraph,sx,sy,0,EAST);
	}else if(sy==(h-1)){
		gg_r(img,newgraph,sx,sy,0,NORTH);
	}else{
		fprintf(stderr,"Start point is not on the edge of the maze...\n");
		return NULL;
	}	
	return newgraph;	
}

void print_graph(graph_t *graph){
	int i;
	int sx, sy;
	int ex, ey;
	int size;

	sx = graph->sx;
	sy = graph->sy;
	ex = graph->ex;
	ey = graph->ey;
	size = graph->size;
	
	printf("Size:\t%d\n",size);
	printf("Start:\t(%d,%d)\n",sx,sy);
	printf("End:\t(%d,%d)\n",ex,ey);

	for(i=0; i<size; i++){
		printf("%d = %d %d %d %d\n", i+1, graph->nodes[i][N1], graph->nodes[i][N2], graph->nodes[i][N3], graph->nodes[i][N4]);
	}

}


int gg_r(	img_t *img,
	       	graph_t *graph,
	       	int x, int y,
	       	int depth,
		int dirs){

	int paths;
	int size;
	int dir;
	int cons = 0;
	int bx,by;


	printf("New Node: (%d,%d) dirs: %d\n",x,y,dirs);
	print_dir(dirs);
	size = graph->size;
	
	graph->nodes[size][CX] = graph->ex;
	graph->nodes[size][CY] = graph->ey;
	graph->nodes[size][ND] = depth;
	if(x==graph->ex && y==graph->ey){
		graph->nodes[size][END] = 1;
		graph->size++;
		return 0;
	}
	if(x==graph->sx && y==graph->sy){
		graph->nodes[size][START] = 1;
		graph->size++;
		return 0;
	}
	graph->size++;
	while(dirs){
		bx = x;
		by = y;
		dir = pop_dir(&dirs);	
		while(1){
			mv_dir(&bx,&by,dir);
			set_dirs(img,&paths,bx,by);
			printf("paths: (%d,%d) %d\n",bx,by,count_dir(paths));
			if(bx==graph->ex && by == graph->ey){
				graph->nodes[size-1][cons] = gg_r(img,graph,bx,by,depth+1,paths);
				cons++;
			}
			if(count_dir(paths)==0){
			       	fprintf(stderr,"WTF: zero paths: (%d,%d)\n",bx,by);
				break;
			}
			if(count_dir(paths)==1){
				switch(paths){
					case NORTH:
						dir = NORTH;
						break;
					case SOUTH:
						dir = SOUTH;
						break;
					case EAST:
						dir = EAST;
						break;
					case WEST:
						dir = WEST;
					default:
						fprintf(stderr,"Direction Failure...\n");
						break;
				}
			}else{
					graph->nodes[size-1][cons] = gg_r(img,graph,bx,by,depth+1,paths);
					cons++;
			}
		}
	}

	return 0;

}
void print_dir(int paths){
	if(is_dir(paths,NORTH)){
		printf("NORTH\n");
	}
	if(is_dir(paths,SOUTH)){
		printf("SOUTH\n");
	}
	if(is_dir(paths,EAST)){
		printf("EAST\n");
	}
	if(is_dir(paths,WEST)){
		printf("WEST\n");
	}
	
}
int pop_dir(int *paths){
	if(is_dir(*paths,NORTH)){
       		*paths = (*paths)^NORTH;
		return NORTH;
	}
	if(is_dir(*paths,SOUTH)){
		*paths = (*paths)^SOUTH;
		return SOUTH;
	}
	if(is_dir(*paths,EAST)){
		*paths = (*paths)^EAST;
		return EAST;
	}
	if(is_dir(*paths,WEST)){
		*paths = (*paths)^WEST;
		return WEST;
	}
	return 0;
}

int is_dir(int p,int d){
//	printf("is_dir %d %d %d\n",p,d,p&d);
	if((p&d)==d) return 1;
	return 0;
}

int count_dir(int paths){
	int count=0;
	if(is_dir(paths,NORTH)) count++;
	if(is_dir(paths,SOUTH)) count++;
	if(is_dir(paths,EAST)) count++;
	if(is_dir(paths,WEST)) count++;
	printf("count: %d\n",count);
	return count;
}

void set_dirs(img_t *img, int *paths, int x, int y){
	*paths = 0;
	if(look_dir(img,x,y,NORTH)) *paths += NORTH;
	if(look_dir(img,x,y,SOUTH)) *paths += SOUTH;
	if(look_dir(img,x,y,EAST)) *paths += EAST;
	if(look_dir(img,x,y,WEST)) *paths += WEST;
}

int look_dir(img_t *img, int x, int y, int dir){
	switch(dir){
		case NORTH:
			if(img->matrix[x][y-1]==PATH){
				return 1;
			}else{
				return 0;
			}
		case SOUTH:
			if(img->matrix[x][y+1]==PATH){
				return 1;
			}else{
				return 0;
			}	
		case EAST:
			if(img->matrix[x+1][y]==PATH){
				return 1;
			}else{
				return 0;
			}
		case WEST:
			if(img->matrix[x-1][y]==PATH){
				return 1;
			}else{
				return 0;
			}
		default:
			fprintf(stderr,"Direction Failure...\n");
			return 0;
	}
}

void mv_dir(int *x, int *y, int dir){
	switch(dir){
		case NORTH:
			(*y)--;
			return;
		case SOUTH:
			(*y)++;
			return;
		case EAST:
			(*x)++;
			return;
		case WEST:
			(*x)--;
			return;
		default:
			fprintf(stderr,"Direction Failure...\n");
			return;;
	}	
}



graph_t *new_graph(img_t *img){
	int **mat;
	int w,h;
	int sx,sy;
	int ex,ey;
	int i,j;
	graph_t *newgraph;

	sx = 0;
	sy = 0;
	ex = 0;
	ey = 0;

	w = img->width;
	h = img->height;
	mat = img->matrix;

	for(i=0; i < w; i++){
		for(j=0; j<h; j++){
			if(mat[i][j] == 100){
				sx = i;
				sy = j;
			}else if(mat[i][j] == 200){
				ex = i;
				ey = j;		
			}
		}
	}

	fprintf(stderr,"Start:\t(%d,%d)\nEnd:\t(%d,%d)\n",sx,sy,ex,ey);

	newgraph = calloc(1,sizeof(graph_t));
	newgraph->nodes = calloc(w*h/4,sizeof(int *));
	newgraph->soln = calloc(w*h/4,sizeof(int));
	for(i=0; i<w*h/4; i++){
		newgraph->nodes[i] = calloc(NODE_SIZE,sizeof(int));
		
	}

	newgraph->sx = sx;
	newgraph->sy = sy;
	newgraph->ex = ex;
	newgraph->ey = ey;
	
	return newgraph;

	
}


void gen_maze(img_t *img,int x, int y, int *count){
	int dir;
	int cn=0;
//	char name[80];
	img->matrix[x][y] = PATH;
//	sprintf(name,"%06d.png",*count);
//	mk_img_img(img,name);
//	(*count)++;
	//printf("(%d,%d)\n",x,y);
	while(cn<1000){
		dir = pow(2,rand()%4);
//		printf("%d\n",dir);

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
			if(col.green == 255 && col.red == 0 && col.blue==0 ){
				newimg->matrix[i][j] = 100;
			}else if(col.red==255 && col.green==0 && col.blue==0){
				newimg->matrix[i][j] = 200;
			}else if(col.red > 128){
				newimg->matrix[i][j] = PATH;
			}else if(col.red < 128){
				newimg->matrix[i][j] = WALL;
			}
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
			if(i==0 && j==1){
				imlib_context_set_color(0,255,0,255);
				imlib_image_draw_pixel(i,j,0);
			}else if(i==w-1 && j == h-2){
				imlib_context_set_color(255,0,0,255);
				imlib_image_draw_pixel(i,j,0);
			}else{
				imlib_context_set_color(val,val,val,255);
				imlib_image_draw_pixel(i,j,0);
			}
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
	
//	x = wrap(x,w);
//	y = wrap(y,h);

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
	newimg->matrix = calloc(w,sizeof(int*));
	for(i=0;i<w;i++){
		newimg->matrix[i] = calloc(h,sizeof(int));
	}
	newimg->width = w;
	newimg->height = h;
	return newimg;

}




