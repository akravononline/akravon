///////////////////////////////////////////////////////////////////////
//         Name: SpeedTreeRT.h
//
//	Cynex: SpeedTreeRT 1.6.0 genel arayuzunun (Aralik 2003) birebir kopyasi.
//	Cynex: uygulama cynex_speedtree (sptconv ciktisi okur); private kisim farkli,
//	Cynex: oyunun eristigi public kisim, enum'lar ve makrolar aynidir.

#pragma once

//  define static or dynamic library build
#ifdef SPEEDTREERT_DYNAMIC_LIB
    #ifdef IDV_SPEEDTREERT_EXPORTS
        #define BUILD_SPEED_TREE_RT_SET __declspec(dllexport)
    #else
        #define BUILD_SPEED_TREE_RT_SET __declspec(dllimport)
    #endif
#else
    #define BUILD_SPEED_TREE_RT_SET
#endif

//  big or little endian system
#ifdef __ppc__
    #define ST_BIG_ENDIAN
#else
    #define ST_LITTLE_ENDIAN
#endif

#include <stddef.h>

struct SCynexSptModel;

class BUILD_SPEED_TREE_RT_SET CSpeedTreeRT
{
public:
        ///////////////////////////////////////////////////////////////////////
        //  Enumerations

        enum EWindMethod
        {
            WIND_GPU, WIND_CPU, WIND_NONE
        };

        enum ELodMethod
        {
            LOD_POP, LOD_SMOOTH, LOD_NONE = 3
        };

        enum ELightingMethod
        {
            LIGHT_DYNAMIC, LIGHT_STATIC
        };

        enum EStaticLightingStyle
        {
            SLS_BASIC, SLS_USE_LIGHT_SOURCES, SLS_SIMULATE_SHADOWS
        };

        enum ECollisionObjectType
        {
            CO_SPHERE, CO_CYLINDER, CO_BOX
        };


        ///////////////////////////////////////////////////////////////////////
        //  SGeometry bit vectors
        //
        //  Passed into GetGeometry() in order to mask out unneeded geometric elements

        #define                 SpeedTree_BranchGeometry            (1 << 0)
        #define                 SpeedTree_FrondGeometry             (1 << 1)
        #define                 SpeedTree_LeafGeometry              (1 << 2)
        #define                 SpeedTree_BillboardGeometry         (1 << 3)
        #define                 SpeedTree_SimpleBillboardOverride   (1 << 4)
        #define                 SpeedTree_AllGeometry               SpeedTree_BranchGeometry + SpeedTree_FrondGeometry + SpeedTree_LeafGeometry + SpeedTree_BillboardGeometry


        ///////////////////////////////////////////////////////////////////////
        //  struct SGeometry declaration

        struct BUILD_SPEED_TREE_RT_SET SGeometry
        {
            SGeometry( );
            ~SGeometry( );

            ///////////////////////////////////////////////////////////////////////
            //  struct SGeometry::SIndexed declaration

            struct BUILD_SPEED_TREE_RT_SET SIndexed
            {
                SIndexed( );
                ~SIndexed( );

                // these values change depending on the active discrete LOD level
                int                     m_nDiscreteLodLevel;    // range: [0, GetNumBranch/FrondLodLevels( ) - 1], -1 if inactive
                unsigned short          m_usNumStrips;          // total number of strips in current LOD
                const unsigned short*   m_pStripLengths;        // lengths of strips in current LOD (m_usNumStrips in length)
                const unsigned short**  m_pStrips;              // triangle strip indices (m_usNumStrips in length)

                // these values are shared across all discete LOD levels
                unsigned short          m_usVertexCount;        // total vertex count in tables, referenced by all LOD levels
                const unsigned long*    m_pColors;              // RGBA values for each leaf - static lighting only (m_usVertexCount in length)
                const float*            m_pNormals;             // normals for each vertex (3 * m_usVertexCount in length)
                const float*            m_pBinormals;           // binormals (bump mapping) for each vertex (3 * m_usVertexCount in length)
                const float*            m_pTangents;            // tangents (bump mapping) for each vertex (3 * m_usVertexCount in length)
                const float*            m_pCoords;              // coordinates for each vertex (3 * m_usVertexCount in length)
                const float*            m_pTexCoords0;          // 1st layer (s,t) texcoords for each vertex (2 * m_usVertexCount in length)
                const float*            m_pTexCoords1;          // 2nd layer (s,t) texcoords for each vertex (2 * m_usVertexCount in length)
                const float*            m_pWindWeights;         // values from from 0.0 for rigid to 1.0 for flexible (m_usVertexCount in length)
                const unsigned char*    m_pWindMatrixIndices;   // table of wind matrix indices (m_usVertexCount in length)
            };


            ///////////////////////////////////////////////////////////////////////
            //  struct SGeometry::SLeaf declaration

            struct BUILD_SPEED_TREE_RT_SET SLeaf
            {
                SLeaf( );
                ~SLeaf( );

                // active LOD level data
                bool                    m_bIsActive;            // flag indicating visibility
                float                   m_fAlphaTestValue;      // 0.0 to 255.0 alpha testing value, used for fading
                int                     m_nDiscreteLodLevel;    // range: [0, GetNumLeafLodLevels( ) - 1]
                unsigned short          m_usLeafCount;          // number of leaves stored in this structure

                // tables for referencing the leaf cluster table
                const unsigned char*    m_pLeafMapIndices;      // references which leaf texture map used for each leaf (m_usLeafCount in length)
                const unsigned char*    m_pLeafClusterIndices;  // references which leaf cluster used for each leaf (m_usLeafCount in length)
                const float*            m_pCenterCoords;        // (x,y,z) values for the centers of leaf clusters (3 * m_usLeafCount in length)
                const float**           m_pLeafMapTexCoords;    // table of unique leaf cluster texcoords (m_usLeafCount in length) - each entry
                                                                // points to 4 pairs of (s,t) texcoords stored in one contiguous array
                const float**           m_pLeafMapCoords;       // table of unique leaf cluster coordinates (m_usLeafCount in length) - each entry
                                                                // points to 4 sets of (x,y,z,0) coordinates stored in one contiguous array
                // remaining vertex attributes
                const unsigned long*    m_pColors;              // RGBA values for each leaf (m_usLeafCount in length)
                const float*            m_pNormals;             // normals for each leaf (3 * m_usLeafCount in length)
                const float*            m_pBinormals;           // binormals (bump mapping) for each leaf (3 * m_usLeafCount in length)
                const float*            m_pTangents;            // tangents (bump mapping) for each leaf (3 * m_usLeafCount in length)
                const float*            m_pWindWeights;         // values from from 0.0 for rigid to 1.0 for flexible (m_usLeafCount in length)
                const unsigned char*    m_pWindMatrixIndices;   // table of wind matrix indices (m_usLeafCount in length)
            };


            ///////////////////////////////////////////////////////////////////////
            //  struct SGeometry::SBillboard declaration

            struct BUILD_SPEED_TREE_RT_SET SBillboard
            {
                SBillboard( );
                ~SBillboard( );

                bool                    m_bIsActive;            // flag indicating visibility
                const float*            m_pTexCoords;           // 4 pairs of (s,t) texcoords stored in one contiguous array
                const float*            m_pCoords;              // 4 sets of (x,y,z) coordindates stored in one contiguous array
                float                   m_fAlphaTestValue;      // 0.0 to 255.0 alpha testing value, used for fading
            };

            ///////////////////////////////////////////////////////////////////////
            //  branch geometry

            SIndexed                m_sBranches;                // holds the branch vertices and index buffers for all
                                                                // of the discrete LOD levels
            float                   m_fBranchAlphaTestValue;    // 0.0 to 255.0 alpha testing value, used for fading

            ///////////////////////////////////////////////////////////////////////
            //  frond geometry

            SIndexed                m_sFronds;                  // holds the frond vertices and index buffers for all
                                                                // of the discrete LOD levels
            float                   m_fFrondAlphaTestValue;     // 0.0 to 255.0 alpha testing value, used for fading

            ///////////////////////////////////////////////////////////////////////
            //  leaf geometry

            SLeaf                   m_sLeaves0;                 // holds the primary leaf geometry, alpha fades into
            SLeaf                   m_sLeaves1;                 // m_sLeaves1 during LOD transitions

            ///////////////////////////////////////////////////////////////////////
            //  billboard geometry

            SBillboard              m_sBillboard0;              // holds the main simple billboard geometry, alpha fades
            SBillboard              m_sBillboard1;              // into m_sBillboard1 in 360 degree mode
            SBillboard              m_sHorizontalBillboard;     // optional horizontal billboard used for aerial views
        };


        ///////////////////////////////////////////////////////////////////////
        //  struct SGeometry::STextures declaration

        struct BUILD_SPEED_TREE_RT_SET STextures
        {
            STextures( );
            ~STextures( );

            // branches
            const char*         m_pBranchTextureFilename;   // null-terminated string

            // leaves
            unsigned int        m_uiLeafTextureCount;       // the number of char* elements in m_pLeafTextureFilenames
            const char**        m_pLeafTextureFilenames;    // array of null-terminated strings m_uiLeafTextureCount in size

            // fronds
            unsigned int        m_uiFrondTextureCount;      // the number of char* elements in m_pFrondTextureFilenames
            const char**        m_pFrondTextureFilenames;   // array of null-terminated strings m_uiFrondTextureCount in size

            // composite
            const char*         m_pCompositeFilename;       // null-terminated string

            // self-shadow
            const char*         m_pSelfShadowFilename;      // null-terminated string
        };


        ///////////////////////////////////////////////////////////////////////
        //  Constructor/Destructor

        CSpeedTreeRT( );
        ~CSpeedTreeRT( );


        ///////////////////////////////////////////////////////////////////////
        //  Memory allocation

static  void*                   operator new(size_t nSize);
static  void*                   operator new[](size_t nSize);
static  void                    operator delete(void* pRawMemory);
static  void                    operator delete[](void* pRawMemory);


        ///////////////////////////////////////////////////////////////////////
        //  Specifying a tree model

        bool                    Compute(const float* pTransform = 0, unsigned int nSeed = 1, bool bCompositeStrips = true);
        CSpeedTreeRT*           Clone(float x = 0.0f, float y = 0.0f, float z = 0.0f, unsigned int nSeed = 0) const;
        const CSpeedTreeRT*     InstanceOf(void) const;
        CSpeedTreeRT*           MakeInstance(void);
        void                    DeleteTransientData(void);

        bool                    LoadTree(const char* pFilename);
        bool                    LoadTree(const unsigned char* pBlock, unsigned int nNumBytes);
        unsigned char*          SaveTree(unsigned int& nNumBytes, bool bSaveLeaves = false) const;

        void                    GetTreeSize(float& fSize, float& fVariance) const;
        void                    SetTreeSize(float fNewSize, float fNewVariance = 0.0f);
        unsigned int            GetSeed( ) const;

        const float*            GetTreePosition(void) const;
        void                    SetTreePosition(float x, float y, float z);

        void                    SetLeafTargetAlphaMask(unsigned char ucMask = 0x54);


        ///////////////////////////////////////////////////////////////////////
        //  Lighting

        //  lighting style
        ELightingMethod         GetBranchLightingMethod(void) const;
        void                    SetBranchLightingMethod(ELightingMethod eMethod);
        ELightingMethod         GetLeafLightingMethod(void) const;
        void                    SetLeafLightingMethod(ELightingMethod eMethod);
        ELightingMethod         GetFrondLightingMethod(void) const;
        void                    SetFrondLightingMethod(ELightingMethod eMethod);

        EStaticLightingStyle    GetStaticLightingStyle(void) const;
        void                    SetStaticLightingStyle(EStaticLightingStyle eStyle);
        float                   GetLeafLightingAdjustment( ) const;
        void                    SetLeafLightingAdjustment(float fScalar);

        //  global lighting state
static  bool                    GetLightState(unsigned int nLightIndex);
static  void                    SetLightState(unsigned int nLightIndex, bool bLightOn);
static  const float*            GetLightAttributes(unsigned int nLightIndex);
static  void                    SetLightAttributes(unsigned int nLightIndex, const float* pLightAttributes);

        //  branch material
        const float*            GetBranchMaterial(void) const;
        void                    SetBranchMaterial(const float* pMaterial);

        //  leaf material
        const float*            GetLeafMaterial(void) const;
        void                    SetLeafMaterial(const float* pMaterial);

        //  frond material
        const float*            GetFrondMaterial(void) const;
        void                    SetFrondMaterial(const float* pMaterial);


        ///////////////////////////////////////////////////////////////////////
        //  Camera

static  void                    GetCamera(float* pPosition, float* pDirection);
static  void                    SetCamera(const float* pPosition, const float* pDirection);


        ///////////////////////////////////////////////////////////////////////
        //  Wind

static  void                    SetTime(float fTime);
        void                    ComputeWindEffects(bool bBranches, bool bLeaves, bool bFronds = true);
        void                    ResetLeafWindState(void);

        bool                    GetLeafRockingState(void) const;
        void                    SetLeafRockingState(bool bFlag);
        void                    SetNumLeafRockingGroups(unsigned int nRockingGroups);

        EWindMethod             GetLeafWindMethod(void) const;
        void                    SetLeafWindMethod(EWindMethod eMethod);
        EWindMethod             GetBranchWindMethod(void) const;
        void                    SetBranchWindMethod(EWindMethod eMethod);
        EWindMethod             GetFrondWindMethod(void) const;
        void                    SetFrondWindMethod(EWindMethod eMethod);

        float                   GetWindStrength(void) const;
        float                   SetWindStrength(float fNewStrength, float fOldStrength = -1.0f, float fFrequencyTimeOffset = -1.0f);

static  void                    SetNumWindMatrices(unsigned int nNumMatrices);
static  void                    SetWindMatrix(unsigned int nMatrixIndex, const float* pMatrix);
        void                    GetLocalMatrices(unsigned int& nStartingIndex, unsigned int& nMatrixSpan);
        void                    SetLocalMatrices(unsigned int nStartingMatrix, unsigned int nMatrixSpan);


        ///////////////////////////////////////////////////////////////////////
        //  LOD

        void                    ComputeLodLevel(void);
        float                   GetLodLevel(void) const;
        void                    SetLodLevel(float fLodLevel);
static  void                    SetDropToBillboard(bool bFlag);

        void                    GetLodLimits(float& fNear, float& fFar) const;
        void                    SetLodLimits(float fNear, float fFar);

        short                   GetDiscreteBranchLodLevel(float fLodLevel = -1.0f) const;
        unsigned short          GetDiscreteLeafLodLevel(float fLodLevel = -1.0f) const;
        short                   GetDiscreteFrondLodLevel(float fLodLevel = -1.0f) const;

        unsigned short          GetNumBranchLodLevels(void) const;
        unsigned short          GetNumLeafLodLevels(void) const;
        unsigned short          GetNumFrondLodLevels(void) const;


        ///////////////////////////////////////////////////////////////////////
        //  Geometry

        void                    DeleteBranchGeometry(void);
        void                    DeleteFrondGeometry(void);
        unsigned char*          GetFrondGeometryMapIndexes(int nLodLevel) const;
        const float*            GetLeafBillboardTable(unsigned int& nEntryCount) const;
        const float*            GetLeafLodSizeAdjustments(void);
        void                    GetGeometry(SGeometry& sGeometry, unsigned long ulBitVector = SpeedTree_AllGeometry, short sOverrideBranchLodValue = -1, short sOverrideFrondLodValue = -1, short sOverrideLeafLodValue = -1);


        ///////////////////////////////////////////////////////////////////////
        //  Textures

        void                    GetTextures(STextures& sTextures) const;
        void                    SetLeafTextureCoords(unsigned int nLeafMapIndex, const float* pTexCoords);
        void                    SetFrondTextureCoords(unsigned int nFrondMapIndex, const float* pTexCoords);
static  bool                    GetTextureFlip(void);
static  void                    SetTextureFlip(bool bFlag);
        void                    SetBranchTextureFilename(const char* pFilename);
        void                    SetLeafTextureFilename(unsigned int nLeafMapIndex, const char* pFilename);
        void                    SetFrondTextureFilename(unsigned int nFrondMapIndex, const char* pFilename);


        ///////////////////////////////////////////////////////////////////////
        //  Statistics & information

static  void                    Authorize(const char* pKey);
static  bool                    IsAuthorized(void);
static  const char*             GetCurrentError(void);
static  void                    ResetError(void);

        void                    GetBoundingBox(float* pBounds) const;
        unsigned int            GetLeafTriangleCount(float fLodLevel = -1.0f) const;
        unsigned int            GetBranchTriangleCount(float fLodLevel = -1.0f) const;
        unsigned int            GetFrondTriangleCount(float fLodLevel = -1.0f) const;


        ///////////////////////////////////////////////////////////////////////
        //  Collision objects

        unsigned int            GetCollisionObjectCount(void);
        void                    GetCollisionObject(unsigned int nIndex, ECollisionObjectType& eType, float* pPosition, float* pDimensions);


        ///////////////////////////////////////////////////////////////////////
        //  User Data

        const char*             GetUserData(void) const;

private:
        CSpeedTreeRT(const CSpeedTreeRT&);
        CSpeedTreeRT&           operator=(const CSpeedTreeRT&);

        // Cynex: paylasilan model (dosya verisi + ornekler arasi ortak durum), ornek basina konum ve LOD
        SCynexSptModel*         m_pModel;
        const CSpeedTreeRT*     m_pInstanceOf;
        float                   m_afPosition[3];
        float                   m_fLodLevel;
};
