/*VICSprites.h*/void	SpriteSetYPosition 				(short	spriteNum);void	SpriteSetXPosition 				(short	spriteNum);void	SpriteEnableDisable 				(void);void	SpriteExpandVertical				(void);void	SpriteExpandHorizontal			(void);void	SpriteSetColor						(short	spriteNum);void	SpriteSetMultiColor 				(void);void	SpriteSetBkgDisplayPriority 	(void);void	SpriteCalculateOneRaster		(short	rasterline);void	SpriteCollideSpriteToSprite 	(unsigned char	sprites);void	SpriteDrawOneRaster				(short	rasterline, unsigned char *dstImagePosition);void	SpriteDrawOneDRaster				(short	rasterline, unsigned char *dstImagePosition);typedef	struct	SpriteData {Rect						bounds;Boolean			multiColor;Boolean			expandVert;Boolean			expandHoriz;Boolean			sprOverBkgPriority;}	SpriteData, *SpritePtr;