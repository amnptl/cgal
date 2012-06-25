#include "GraphicsViewCurveInput.h"
#include <QGraphicsView>

namespace CGAL {
namespace Qt {

void 
GraphicsViewCurveInputBase::
setScene( QGraphicsScene* scene_ )
{
    this->scene = scene_;
}

QGraphicsScene* 
GraphicsViewCurveInputBase::
getScene( ) const
{
    return this->scene;
}

void
GraphicsViewCurveInputBase::
setSnappingEnabled( bool b )
{
    this->snappingEnabled = b;
}

void
GraphicsViewCurveInputBase::
setSnapToGridEnabled( bool b )
{
    this->snapToGridEnabled = b;
}

GraphicsViewCurveInputBase::
GraphicsViewCurveInputBase( QObject* parent ):
    GraphicsViewInput( parent ),
    scene( NULL ),
    snappingEnabled( false ),
    snapToGridEnabled( false )
{ }

void 
GraphicsViewCurveInputBase::
mouseMoveEvent( QGraphicsSceneMouseEvent* event )
{ }

void 
GraphicsViewCurveInputBase::
mousePressEvent( QGraphicsSceneMouseEvent* event )
{ 
    std::cout << "GraphicsViewCurveInputBase::mousePressEvent" << std::endl;
}

bool 
GraphicsViewCurveInputBase::
eventFilter( QObject* obj, QEvent* event )
{
    if ( event->type( ) == QEvent::GraphicsSceneMouseMove )
    {
        QGraphicsSceneMouseEvent* mouseEvent =
            static_cast< QGraphicsSceneMouseEvent* >( event );
        this->mouseMoveEvent( mouseEvent );
    }
    else if ( event->type( ) == QEvent::GraphicsSceneMousePress )
    {
        QGraphicsSceneMouseEvent* mouseEvent =
            static_cast< QGraphicsSceneMouseEvent* >( event );
        this->mousePressEvent( mouseEvent );
    }

    return QObject::eventFilter( obj, event );
}

QRectF
GraphicsViewCurveInputBase::
viewportRect( ) const
{
    QRectF res;
    if ( this->scene == NULL )
    {
        return res;
    }

    QList< QGraphicsView* > views = this->scene->views( );
    if ( views.size( ) == 0 )
    {
        return res;
    }
    // assumes the first view is the right one
    QGraphicsView* viewport = views.first( );
    QPointF p1 = viewport->mapToScene( 0, 0 );
    QPointF p2 = viewport->mapToScene( viewport->width( ), viewport->height( ) );
    res = QRectF( p1, p2 );

    return res;
}

} // namespace Qt
} // namespace CGAL
