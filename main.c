
/*
** Sega Dreamcast GL / LibXVID Example (C) Josh PH3NOM Pearson 2013
*/

#include <stdio.h>

#include "gl/gl.h"

#include "LibXVID.h"

#include "container.h"
#include "cstring.h"
#include "font.h"
#include "fs.h"
#include "input.h"
#include "texture.h"

pvr_init_params_t params = {
    /* Enable opaque and translucent polygons with size 16 */
    { PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_0 },

    /* Vertex buffer size 2Mb */
    512*1024*4
};

Font * LoadFontTex();

String * FileSelect( Font * font, String *dir );

void RenderErrorScreen( Font * font, char * file_name ); 

static GLuint BG, TBN=0;

int main()
{
     printf("LibXviD/OpenGL 2D Test v.0.0.3 (C) Josh PH3NOM Pearson 2013\n");

     pvr_init(&params);  /* Init the PVR */

     glKosInit();  /* Init the GL */
	 
	 /* Set all Gl transform matrices to identity for 2D transform */
	 glMatrixMode(GL_SCREENVIEW);
	 glLoadIdentity();
	 glMatrixMode(GL_PROJECTION);
	 glLoadIdentity();
	 glMatrixMode(GL_MODELVIEW);
	 glLoadIdentity();    
     
     Font * font = LoadFontTex();
     String * FileName;
     String * DirName = NewString();
     StringSet( DirName, "\0" );

while(1)
{     
     FileName = FileSelect( font, DirName );
     
     printf("FileExplorer v.1.0 Open DIR: %s\n", DirName->String );
     printf("File Selected: %s\n", FileName->String );

     switch( FsMediaContainer( FileName->String ) )
     {
         case CONTAINER_AVI:
              LibXVID_Decode( FileName->String, font );
              break;
              
         case CONTAINER_MPG:
              LibMPEG2_Decode( FileName->String, font );
              break;
              
         case CONTAINER_UNKNOWN:
              printf("UnKnown(NULL) File Type\n");
              RenderErrorScreen( font, FileName->String);
              break;    
     }

     StringDeconstruct( FileName );
}
     return 1;
}

void RenderBG()
{
    glBindTexture( GL_TEXTURE_2D, BG );  
    
    glBegin(GL_QUADS);
    
    glTexCoord2f( 0, 1 );
    glVertex3f( 80, 480, 1.0f );
        
    glTexCoord2f( 1, 1 );
    glVertex3f( 560, 480, 1.0f );
        
    glTexCoord2f( 1, 0 );
    glVertex3f( 560, 0, 1.0f );
        
    glTexCoord2f( 0, 0 );
    glVertex3f( 80, 0, 1.0f );      
       
    glEnd();
}

void RenderQuad( float x, float y, float w, float h, uint32 color )
{
    glColor1ui( color );  
    glVertex3f( x, y, 1.f );
    glVertex3f( x, y+h, 1.f );     
    glVertex3f( x+w, y+h, 1.f );    
    glVertex3f( x+w, y, 1.f );
}

void RenderTbn( VideoThumbnail * tbn, float x, float y, float w, float h )
{
    if(tbn!=NULL)
        glBindTexture( GL_TEXTURE_2D, TBN );  
    else
        glBindTexture( GL_TEXTURE_2D, BG );  
    
    glBegin(GL_QUADS);
    
    glTexCoord2f( 0, 1 );
    glVertex3f( x, y+h, 1.0f );
        
    glTexCoord2f( 1, 1 );
    glVertex3f( x+w, y+h, 1.0f );
        
    glTexCoord2f( 1, 0 );
    glVertex3f( x+w, y, 1.0f );
        
    glTexCoord2f( 0, 0 );
    glVertex3f( x, y, 1.0f );      
       
    glEnd();     
}

void RenderErrorScreen( Font * font, char * file_name )
{
    String * ErrMsg = NewString();
    StringSet( ErrMsg, file_name );
    StringInsert( ErrMsg, "File UnSupported: \n" );
        
    glKosBeginFrame();
        
    RenderBG();

    glBindTexture( GL_TEXTURE_2D, font->TexId );  
    
    glBegin(GL_QUADS);
    
    float w = 12.0f, width = (ErrMsg->Size-1)*w;
    float x = (vid_mode->width/2.0f)-(width/2.0f);
        
    FontPrintString( font, ErrMsg->String, x, 200.f, w, w );

    glEnd();
     
	glKosFinishFrame(); 
    
    StringDeconstruct( ErrMsg );
    
    thd_sleep(5000);
}

void RenderDir( DirEntry * dir, Font * font, int entry, int entries )
{
    float x,y=60.0f,w=14.0f,h=14.0f;
    unsigned int i;
    DirEntry *dptr = dir;
    
    glBindTexture( GL_TEXTURE_2D, font->TexId );  
    
    glBegin(GL_QUADS);
    
    for(i=0;i<entries;i++)
    {
        float width = cstr_len(dptr->Name)*w;
        x = (vid_mode->width/2.0f)-(width/2.0f);
         
        FontPrintString( font, dptr->Name, x, y, w, h );
         
        if(entry==i)
        {
            RenderQuad( x-w-w, y, w, h, 0xFF00FF00 );
            RenderQuad( x+width+1.0f, y, w, h, 0xFF00FF00 );
        }
         
        y+=h+1.0f; ++dptr;
    }
    
    glEnd();
}

Font * LoadFontTex()
{
    TexStruct *texture[2]; 

    texture[0] = TexturePVR( "/cd/base0.pvr" );
    texture[1] = TexturePVR( "/cd/wmp.pvr" );
      
    printf("Font: %ix%i\n", texture[0]->w, texture[0]->h ); 
    printf("Bgnd: %ix%i\n", texture[1]->w, texture[1]->h ); 
    
    Font *font = FontInit( texture[0]->data, texture[0]->fmt | texture[0]->col,
                           texture[0]->w, texture[0]->h, 10, 10,
                           PVR_PACK_COLOR( 1.0f, 0.0f, 0.1f, 1.0f ) );

    glGenTextures(1, &BG);
    glBindTexture(GL_TEXTURE_2D, BG );
    glKosTex2D( texture[1]->fmt | texture[1]->col,
                texture[1]->w, texture[1]->h,
                texture[1]->data );
                                              
    glKosBeginFrame();
        
    RenderBG();

    glBindTexture( GL_TEXTURE_2D, font->TexId );  
    
    glBegin(GL_QUADS);

    FontPrintString( font,  "LibXVID-DC (C) PH3NOM 2013", 100.f, 200.f, 16.f, 16.f );

    glEnd();
     
	glKosFinishFrame();  

    glKosBeginFrame();
        
    RenderBG();

    glBindTexture( GL_TEXTURE_2D, font->TexId );  
    
    glBegin(GL_QUADS);

    FontPrintString( font,  "LibXVID-DC (C) PH3NOM 2013", 100.f, 200.f, 16.f, 16.f );

    glEnd();
     
	glKosFinishFrame();  
   
    thd_sleep(3000);

    return font;
}

String * FileSelect( Font * font, String * directory )
{
     unsigned int frame=0,fs_entries,fs_entry=0,i;
     DirEntry *dir,*dptr;
     Input input;

     String * FileName = NewString();
     
     printf("FileSelect: LoadDir %s\n", directory->String );
               
     dir = FsParseDirectory( directory->String, &fs_entries );
    
   renderdir:
               
     DCE_SetInput( &input, 0 );
     
     while(!input.a)
     {
        glKosBeginFrame();
        
        RenderBG();

        RenderDir( dir, font, fs_entry, fs_entries );
     
	    glKosFinishFrame();  
	
	    if(++frame%3==0)
	    {
           DCE_SetInput( &input, 0 );
	       DCE_GetInput( &input );
	       
	       if(input.u)
	           if(fs_entry>0)
	              --fs_entry;
            if(input.d)
               if(fs_entry<fs_entries-1)
                  ++fs_entry;
        }
    }
    
    if(!fs_entries) // Selected No File ... Goto root dir
    {
        FsFreeDirectory( dir, fs_entries );
        StringDeconstruct(FileName);
        StringSet( directory, "\0" );
        return FileSelect( font, directory ); // Recursion :0
    }
    
    dptr = dir+fs_entry;

    StringSet( FileName, dptr->Name );
    StringInsert( FileName, directory->String );
    
    if(!FsIsFile(FileName->String))
    {
        unsigned char RootDir = FsFixDir( FileName );

        FsFreeDirectory( dir, fs_entries );
        
        StringSet( directory, FileName->String );
        if(!RootDir) StringConcat( directory, "/\0");

        dir = FsParseDirectory( directory->String, &fs_entries );
        
        fs_entry=0;
        goto renderdir;
    }
    
    FsFreeDirectory( dir, fs_entries );
    
    return FileName;
}
