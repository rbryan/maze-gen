struct img_t{
	int width, height;
	int **matrix;
};
typedef struct img_t img_t;

struct graph_t{
	int sx,sy;
	int ex,ey;
	int size;
	int **nodes;
	int *soln;
};
typedef struct graph_t graph_t;


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
graph_t *get_graph(img_t *img);
int gg_r(	img_t *img,
	       	graph_t *graph,
	       	int x, int y,
	       	int depth,
	       	int dirs);
graph_t *new_graph(img_t *img);
void mv_dir(int *x, int *y, int dir);
int look_dir(img_t *img, int x, int y, int dir);
void set_dirs(img_t *img, int *paths, int x, int y);
int is_dir(int p,int d);
int pop_dir(int *paths);
int count_dir(int paths);
void print_graph(graph_t *graph);
