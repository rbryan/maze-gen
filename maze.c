#include "maze.h"
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <Imlib2.h>
#include <pthread.h>
#include <sys/resource.h>


#define MIN(X,Y)	((X < Y) ? (X) : (Y))
#define MAX(X,Y)	((X > Y) ? (X) : (Y))

#define NORTH	1
#define SOUTH	2
#define EAST	4
#define WEST	8

#define PATH	255
#define WALL	0
#define MARKED  128


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
			exit(1);
		}
		set_stack_size(atoi(argv[5]));	
	
		srand(time(NULL));
		img = new_img(2*atoi(argv[2])+1, 2*atoi(argv[3])+1);
		gen_maze(img,3,3, NULL);
		mk_img_img(img,argv[4]);
		return 0;
	}else if(*argv[1] == 's'){
		if(argc < 4){
			fprintf(stderr,"Usage: maze s <img> <output>\n");
			exit(1);
		}
		img_t *img,*trimg;
		int cx=0,cy=0;
		tdata data;
		pthread_t trim_thread;
		pthread_t solve_thread;
		pthread_mutex_t lock;

		pthread_mutex_init(&lock,NULL);

		
		img = ld_img_img(argv[2]);
		cp_img(&trimg,img);
		
		data.cx = &cx;
		data.cy = &cy;
		data.real = img;
		data.trimmed = trimg;
		data.lock = &lock;

		pthread_create(&trim_thread,NULL,trim,&data);
		pthread_create(&solve_thread,NULL,lhsolve,&data);
		
		//pthread_detach(trim_thread);
		pthread_join(trim_thread,NULL);
		mk_img_img(trimg,"trimmed.png");
		pthread_join(solve_thread,NULL);
		mk_img_img(img,argv[3]);
		mk_img_img(trimg,"trimmed.png");

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


void cp_img(img_t **dest, img_t *src){
	int w,h;
	int i,j;
	
	w = src->width;
	h = src->height;

	*dest = new_img(w,h);

	(*dest)->width = w;
	(*dest)->height = h;
	(*dest)->sx = src->sx;
	(*dest)->sy = src->sy;
	(*dest)->ex = src->ex;
	(*dest)->ey = src->ey;


	for(i=0; i < w; i++){
		for(j=0; j < h; j++){
			(*dest)->matrix[i][j] = src->matrix[i][j];
		}
	}

}

void * trim(void *data){
	img_t *real;
	img_t *trimmed;
	int ex,ey;
	int sx,sy;
	int *cx,*cy;
	int trim_f;
	int i,j;
	int w,h;
	int **tmat;
	int dirs;
	pthread_mutex_t *lock;

	real = ((tdata *) data)->real;
	trimmed = ((tdata *) data)->trimmed;
	cx = ((tdata *) data)->cx;
	cy = ((tdata *) data)->cy;
	lock = ((tdata *) data)->lock;

	tmat = trimmed->matrix;

	ex = real->ex;
	ey = real->ey;

	sx = real->sx;
	sy = real->sy;

	w = real->width;
	h = real->height;

	trim_f=1;


	while(trim_f){
		trim_f=0;
		for(i=1; i<w; i+=2){
			for(j=1; j<h; j+=2){
				dirs = get_dirs(trimmed,i,j);
				if(count_dir(dirs)==1){
					pthread_mutex_lock(lock);
					if(	(i != ex || j != ey) &&
					       	(i != *cx || j != *cy) &&
					       	(i != sx || j != sy)){
						fill(tmat,i,j,dirs);
						trim_f++;
					}
					pthread_mutex_unlock(lock);
				}	
			}
		}
	}
	

	return NULL;
}



inline void fill(int **mat,int x, int y, int dir){

	mat[x][y] = WALL;

	switch(dir){
		case NORTH:
			mat[x][y-1]=WALL;
			return;
		case SOUTH:
			mat[x][y+1]=WALL;
			return;
		case EAST:
			mat[x+1][y]=WALL;
			return;
		case WEST:
			mat[x-1][y]=WALL;
			return;
		default:
			fprintf(stderr,"Direction Failure...\n");
			exit(2);
	}

}


void * lhsolve(void *data){
	img_t *real;
	img_t *trimmed;
	int ex,ey;
	int dir;
	int *cx,*cy;
	int ds,dr,dl;
//	pthread_mutex_t *lock;
	struct timespec start,end;

	dir = 1;

	real = ((tdata *) data)->real;
	trimmed = ((tdata *) data)->trimmed;
	cx = ((tdata *) data)->cx;
	cy = ((tdata *) data)->cy;
//	lock = ((tdata *) data)->lock;

	ex = real->ex;
	ey = real->ey;

	*cx = real->sx;
	*cy = real->sy;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&start);

	while(*cx != ex || *cy != ey){
		ds = look_dir(trimmed,*cx,*cy,dir);
		dr = look_dir(trimmed,*cx,*cy,turn_right(dir));
		dl = look_dir(trimmed,*cx,*cy,turn_left(dir));

		if(dl){
			dir = turn_left(dir);
		}else if(ds){
			
		}else if(dr){
			dir = turn_right(dir);
		}else{
			dir = turn_180(dir);
		}
		mv_dir(real,cx,cy,dir);
	}
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&end);
	printf("Start Time: %ld:%ld\n",start.tv_sec,start.tv_nsec);
	printf("End Time: %ld:%ld\n",end.tv_sec,end.tv_nsec);
	printf("Diff: %ld:%ld\n",end.tv_sec-start.tv_sec,end.tv_nsec-start.tv_nsec);
	return NULL;
}

int seen(img_t *img,int x,int y){
	if(img->matrix[x][y] == MARKED){
		return 1;
	}
	return 0;
}

void toggle_seen(img_t *img,int x, int y){
	if(img->matrix[x][y] == MARKED){
		img->matrix[x][y] = PATH;
		return;
	}
	if(img->matrix[x][y] == PATH){
		img->matrix[x][y] = MARKED;
		return;
	}
}

int turn_180(int dir){
	
	switch(dir){
		case NORTH:
			return SOUTH;
		case SOUTH:
			return NORTH;
		case EAST:
			return WEST;
		case WEST:
			return EAST;
		default:
			fprintf(stderr,"Direction Failure...\n");
			exit(2);
	}

}

int turn_right(int dir){

	switch(dir){
		case NORTH:
			return WEST;
		case SOUTH:
			return EAST;
		case EAST:
			return NORTH;
		case WEST:
			return SOUTH;
		default:
			fprintf(stderr,"Direction Failure...\n");
			exit(2);
	}

}

int turn_left(int dir){
	switch(dir){
		case NORTH:
			return EAST;
		case SOUTH:
			return WEST;
		case EAST:
			return SOUTH;
		case WEST:
			return NORTH;
		default:
			fprintf(stderr,"Direction Failure...\n");
			exit(2);
	}
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
	if((p&d)==d) return 1;
	return 0;
}

int count_dir(int paths){
	int count=0;
	if(is_dir(paths,NORTH)) count++;
	if(is_dir(paths,SOUTH)) count++;
	if(is_dir(paths,EAST)) count++;
	if(is_dir(paths,WEST)) count++;
	return count;
}

int get_dirs(img_t *img, int x, int y){
	int paths = 0;
	if(look_dir(img,x,y,NORTH)) paths = paths|NORTH;
	if(look_dir(img,x,y,SOUTH)) paths = paths|SOUTH;
	if(look_dir(img,x,y,EAST)) paths = paths|EAST;
	if(look_dir(img,x,y,WEST)) paths = paths|WEST;
	return paths;
}

int look_dir(img_t *img, int x, int y, int dir){
	switch(dir){
		case NORTH:
			if(img->matrix[x][y-1]==PATH || img->matrix[x][y-1]==MARKED){
				return 1;
			}else{
				return 0;
			}
		case SOUTH:
			if(img->matrix[x][y+1]==PATH || img->matrix[x][y+1]==MARKED){
				return 1;
			}else{
				return 0;
			}	
		case EAST:
			if(img->matrix[x+1][y]==PATH || img->matrix[x+1][y]==MARKED){
				return 1;
			}else{
				return 0;
			}
		case WEST:
			if(img->matrix[x-1][y]==PATH || img->matrix[x-1][y]==MARKED){
				return 1;
			}else{
				return 0;
			}
		default:
			fprintf(stderr,"Direction Failure...\n");
			exit(2);
	}
}

void mv_dir(img_t *img, int *x, int *y, int dir){
	switch(dir){
		case NORTH:
			(*y)-=2;
			toggle_seen(img,*x,*y+1);
			return;
		case SOUTH:
			(*y)+=2;
			toggle_seen(img,*x,*y-1);
			return;
		case EAST:
			(*x)+=2;
			toggle_seen(img,*x-1,*y);
			return;
		case WEST:
			(*x)-=2;
			toggle_seen(img,*x+1,*y);
			return;
		default:
			fprintf(stderr,"Direction Failure...\n");
			exit(2);
	}	
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
	printf("Loading Image: %s\n",name);
	image = imlib_load_image(name);
	imlib_context_set_image(image);

	w = imlib_image_get_width();
	h = imlib_image_get_height();


	newimg = new_img(w,h);

	for(i=0; i<w; i++){
		for(j=0; j<h; j++){
			imlib_image_query_pixel(i,j,&col);
			if(col.green == 255 && col.red == 0 && col.blue==0 ){
				newimg->matrix[i][j] = WALL;
				newimg->sx = i;
				newimg->sy = j;
			}else if(col.red==255 && col.green==0 && col.blue==0){
				newimg->matrix[i][j] = WALL;
				newimg->ex = i;
				newimg->ey = j;
			}else if(col.red > 128){
				newimg->matrix[i][j] = PATH;
			}else if(col.red < 128){
				newimg->matrix[i][j] = WALL;
			}
		}
	}
	if(newimg->sx ==0){
		newimg->sx = 1;
	}
	if(newimg->ex ==0){
		newimg->ex = 1;
	}
	if(newimg->sy == 0){
		newimg->sy = 1;
	}
	if(newimg->ey == 0){
		newimg->ey = 1;
	}
	if(newimg->sx ==newimg->width-1){
		newimg->sx = newimg->sx-1;
	}
	if(newimg->ex ==newimg->width-1){
		newimg->ex = newimg->ex-1;
	}
	if(newimg->sy == newimg->height-1){
		newimg->sy = newimg->sy-1;
	}
	if(newimg->ey == newimg->height-1){
		newimg->ey = newimg->ey-1;
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
	int sx,sy;
	int ex,ey;

	sx = img->sx;
	sy = img->sy;
	ex = img->ex;
	ey = img->ey;


	mat = img->matrix;

	w = img->width;
	h = img->height;
	
	if(sx == 0 && sy == 0){
		sx = 0;
		sy = 1;
		ex = w-1;
		ey = h-2;
	}

	image = imlib_create_image(w,h);

	imlib_context_set_image(image);

	for(i=0; i < w; i++){
		for(j=0; j < h; j++){
			val = mat[i][j];
			if(i==sx && j==sy){
				imlib_context_set_color(0,255,0,255);
			}else if(i==ex && j == ey){
				imlib_context_set_color(255,0,0,255);
			}else if(img->matrix[i][j] == MARKED){
				imlib_context_set_color(0,0,255,255);		
			}else{
				imlib_context_set_color(val,val,val,255);
			}
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




