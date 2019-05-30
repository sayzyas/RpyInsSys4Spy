#ifndef _DEBUG_H_
#define _DEBUG_H_




#define __DEBUG_MENU__
//#define __LEVEL0__
//#define __LEVEL1__
//#define __LEVEL2__
//#define __LEVEL3__
//#define __LEVEL4__
//#define __LEVEL5__

#ifdef __DEBUG_MENU__
    #define debug_menu(...)  fprintf( stderr, __VA_ARGS__ )
#else
    #define debug_menu(...) 
#endif

#ifdef __ADMIN_MODE__
    #define admin_mode(...)  fprintf( stderr, __VA_ARGS__ )
#else
    #define admin_mode(...) 
#endif

#ifdef __LEVEL0__
    #define debug0(...)  fprintf( stderr, __VA_ARGS__ )
#else
    #define debug0(...) 
#endif
#ifdef __LEVEL1__
    #define debug1(...)  fprintf( stderr, __VA_ARGS__ )
#else
    #define debug1(...) 
#endif
#ifdef __LEVEL2__
    #define debug2(...)  fprintf( stderr, __VA_ARGS__ )
#else
    #define debug2(...) 
#endif
#ifdef __LEVEL3__
    #define debug3(...)  fprintf( stderr, __VA_ARGS__ )
#else
    #define debug3(...) 
#endif
#ifdef __LEVEL4__
    #define debug4(...)  fprintf( stderr, __VA_ARGS__ )
#else
    #define debug4(...) 
#endif
#ifdef __LEVEL5__
    #define debug5(...)  fprintf( stderr, __VA_ARGS__ )
#else
    #define debug5(...) 
#endif


/* MACROS */

#define ADMIN_PRINT( flg, cmd, fname ) \
  do { \
    char *cbuf; \
  	cbuf = (char*)malloc( 256 ); \
    if( flg == true ){ \
      fprintf( stderr, cmd ); \
      strcpy( cbuf, "echo '"); \
      strcat( cbuf, cmd ); \
      strcat( cbuf, " ' >> " ); \
      strcat( cbuf, fname ); \
      system( cbuf ); \
      fprintf( stderr, "\n"); \
    } \
  	else \
  	{ \
  	  fprintf( stderr, cmd ); \
  	  fprintf( stderr, "\n"); \
  	} \
  	free(cbuf); \
  } while (0)

#define ADMIN_PRINT2( flg, cmd, fname ) \
  do { \
    char *cbuf; \
  	cbuf = (char*)malloc( 256 ); \
    if( flg == true ){ \
      fprintf( stderr, cmd ); \
      strcpy( cbuf, "echo '"); \
      strcat( cbuf, cmd ); \
      strcat( cbuf, " ' >> " ); \
      strcat( cbuf, fname ); \
      system( cbuf ); \
      fprintf( stderr, "\n"); \
    } \
  	else \
  	{ \
  	  fprintf( stderr, cmd ); \
  	  fprintf( stderr, "\n"); \
  	} \
  	free(cbuf); \
  } 


/*

strcmp(s1,s2)==0

		strcpy( rout, "echo ADMINISTRATOR MODE > " );
		strcat( rout, argv[1] ); 
		system( rout );
		admin_mode( rout );	
*/

#endif

