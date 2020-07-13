


typedef enum
{
    SVG_GROUP = 0,
    SVG_RECT = 1,
    SVG_CIRCLE = 2,
    SVG_ELLIPSE = 3,
    SVG_LINE = 4,
} svg_NodeType;

typedef struct
{
    svg_NodeType Type;
    void *pData;
    svg_Node *pParent;
    svg_Node **ppChildren;
    unsigned int Child_Count;
} svg_Node;