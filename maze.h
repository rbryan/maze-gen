struct img_t{
	int width, height;
	int **matrix;
};
typedef struct img_t img_t;


img_t *new_img(int w,int h);

void set_val(img_t *img, int val, int x, int y);

int get_val(img_t *img, int x, int y);
void free_img(img_t *img);

img_t *ld_img_img(const char *name);
void mk_img_img(img_t *img, const char *name);
inline int wrap(register int x, register int side);
void gen_maze(img_t *img,int x, int y, int *count);
