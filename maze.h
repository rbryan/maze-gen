#include <pthread.h>

struct img_t{
	int width, height;
	int **matrix;
	int sx,sy;
	int ex,ey;
};
typedef struct img_t img_t;

struct tdata{
	img_t *real;
	img_t *trimmed;
	int *cx,*cy;
	pthread_mutex_t *lock;
};
typedef struct tdata tdata;

img_t *new_img(int w,int h);

void set_val(img_t *img, int val, int x, int y);

int get_val(img_t *img, int x, int y);
void free_img(img_t *img);

img_t *ld_img_img(const char *name);
void mk_img_img(img_t *img, const char *name);
inline int wrap(register int x, register int side);
int chk_pt(img_t *img, int x, int y);
void gen_maze(img_t *img,int x, int y, int *count);
void set_stack_size(int mb);
void mv_dir(img_t * img,int *x, int *y, int dir);
int look_dir(img_t *img, int x, int y, int dir);
int is_dir(int p,int d);
int pop_dir(int *paths);
int count_dir(int paths);
void print_dir(int paths);
void * solv_maze(void *data);
void cp_img(img_t **dest, img_t *src);
int get_dirs(img_t *img, int x, int y);
void toggle_seen(img_t *img,int x, int y);
int turn_180(int dir);
int turn_right(int dir);
int turn_left(int dir);
void * lhsolve(void *data);
int seen(img_t *img,int x,int y);
inline void fill(int **mat,int x, int y, int dir);
void * trim(void *data);
