#include <sys/types.h>
#include <dirent.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>

#include </usr/include/jpeglib.h>
#include <unistd.h>

#include <setjmp.h>
#include <err.h>

struct my_error_mgr {
	struct jpeg_error_mgr pub;
	jmp_buf setjmp_buffer;
};
typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
	my_error_ptr myerr = (my_error_ptr) cinfo->err;

	//if(	cinfo->err->msg_code != 52)
		(*cinfo->err->output_message) (cinfo);
	longjmp(myerr->setjmp_buffer, 1);
}

METHODDEF(void)
my_emit_message(j_common_ptr cinfo, int msg_level)
{
	my_error_ptr myerr = (my_error_ptr) cinfo->err;
	if (msg_level == -1)
	{
		(*cinfo->err->output_message) (cinfo);
		longjmp(myerr->setjmp_buffer, 1);
	}
}

void SaveFile(FILE *fd, size_t pos)
{
	size_t end = ftell(fd);
	fseek(fd,pos-3,SEEK_SET);
	char name[100];
	{
		memset(name,0,100);
		static int a=0;
		a++;
		sprintf(name,"%d.jpg",a);
	}
	FILE *nf = fopen(name,"wab");
	if (nf == (void*)-1)
	{
		printf("fail to create new jpg file!!!");
		exit(1);
	}

	while(pos < end)
	{
		pos++;
		int8_t b;
		fread(&b,1,1,fd);
		fwrite(&b,1,1,nf);
	}

	fclose(nf);
}

void TestJpg(FILE *fd, size_t pos )
{
	fseek(fd,pos-3,SEEK_SET);

	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	jerr.pub.emit_message = my_emit_message	;
	if (setjmp(jerr.setjmp_buffer)) {
		jpeg_destroy_decompress(&cinfo);
		//printf("errNum:%d",cinfo.err->msg_code);
		//if(cinfo.err->msg_code != 53)
			SaveFile(fd,pos);
		rewind(fd);
		fseek(fd,pos,SEEK_SET);
		return;
	}
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, fd);

	jpeg_read_header(&cinfo, FALSE);
	if( cinfo.global_state == 200)
	{
		rewind(fd);
		fseek(fd,pos,SEEK_SET);
		return;
	}

	jpeg_start_decompress(&cinfo);

	int row_stride = cinfo.output_width * cinfo.output_components;
	JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	while (cinfo.output_scanline < cinfo.output_height) {
		(void) jpeg_read_scanlines(&cinfo, buffer, 1);
		//put_scanline_someplace(buffer[0], row_stride);
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	SaveFile(fd,pos);
	rewind(fd);
	fseek(fd,pos,SEEK_SET);
}

int main(int argc, char**argv)
{
	FILE *fd = fopen(argv[1], "rb");

	if(fd == (void*)-1)
	{
		err(1, "%s", argv[1]);
		return 1;
	}

	int8_t buff[3] = {0,0,0};

	fseek(fd,0,SEEK_END);
	size_t fsize = ftell(fd);
	fseek(fd,0,SEEK_SET);

	int16_t cnt;

	for(size_t fpos=0;fpos<=fsize;fpos++)
	{
		if (ftell(fd) != fpos)
			fpos =12;

		if(cnt++==0)
			printf("\r%lf %%",100*((double)fpos/(double)fsize));
		buff[0] = buff[1];
		buff[1] = buff[2];
		fread(&(buff[2]), 1, 1, fd);
		if (	buff[0]==(int8_t)0xFF &&
				buff[1]==(int8_t)0xD8 &&
				buff[2]==(int8_t)0xFF)
			TestJpg(fd,fpos+1);
	}

	fclose(fd);
	return 0;
}
