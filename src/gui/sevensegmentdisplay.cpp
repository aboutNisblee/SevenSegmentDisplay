/**
 * \file sevensegmentdisplay.cpp
 *
 * \date 04.09.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include <gui/sevensegmentdisplay.hpp>
#include <gui/displaynode_p.hpp>

struct SevenSegmentDisplayPrivate
{
	SevenSegmentDisplayPrivate():
		mDisplayNode(new DisplayNode)
	{
	}
	Q_DISABLE_COPY(SevenSegmentDisplayPrivate)

	// Owned by scene graph
	DisplayNode* mDisplayNode;
};

SevenSegmentDisplay::SevenSegmentDisplay(QQuickItem* parent) :
	QQuickItem(parent), d_ptr(new SevenSegmentDisplayPrivate())
{
	setFlag(ItemHasContents, true);
	//	connect(this, &QQuickItem::widthChanged, this, [&]() { qDebug() << "width:" << width(); });
	//	connect(this, &QQuickItem::heightChanged, this, [&]() { qDebug() << "height:" << height(); });
}

SevenSegmentDisplay::~SevenSegmentDisplay()
{
}

int SevenSegmentDisplay::getDigitCount() const { Q_D(const SevenSegmentDisplay); return d->mDisplayNode->getDigitCount(); }
void SevenSegmentDisplay::setDigitCount(int count)
{
	Q_D(SevenSegmentDisplay);
	if(count < 0)
		qWarning() << "Digit count cannot be negative";
	else if (d->mDisplayNode->setDigitCount(count))
	{
		update();
		emit digitCountChanged();
	}
}

double SevenSegmentDisplay::getValue() const { Q_D(const SevenSegmentDisplay); return d->mDisplayNode->getValue(); }
void SevenSegmentDisplay::setValue(double value)
{
	Q_D(SevenSegmentDisplay);
	if(d->mDisplayNode->setValue(value))
	{
		update();
		emit valueChanged();
	}
}

int SevenSegmentDisplay::getPrecision() const { Q_D(const SevenSegmentDisplay); return d->mDisplayNode->getPrecision(); }
void SevenSegmentDisplay::setPrecision(int precision)
{
	Q_D(SevenSegmentDisplay);
	if(precision < 0)
		qWarning() << "Precision cannot be negative";
	else if(d->mDisplayNode->setPrecision(precision))
	{
		update();
		emit precisionChanged();
	}
}

int SevenSegmentDisplay::getDigitSize() const { Q_D(const SevenSegmentDisplay); return d->mDisplayNode->getDigitSize(); }
void SevenSegmentDisplay::setDigitSize(int size)
{
	Q_D(SevenSegmentDisplay);
	if(d->mDisplayNode->setDigitSize(size))
	{
		update();
		emit digitSizeChanged();
	}
}

SevenSegmentDisplay::Alignment SevenSegmentDisplay::getVerticalAlignment() const { Q_D(const SevenSegmentDisplay); return d->mDisplayNode->getVAlignment(); }
void SevenSegmentDisplay::setVerticalAlignment(Alignment alignment)
{
	Q_D(SevenSegmentDisplay);
	if(d->mDisplayNode->setVAlignment(alignment))
	{
		update();
		emit verticalAlignmentChanged();
	}
}

SevenSegmentDisplay::Alignment SevenSegmentDisplay::getHorizontalAlignment() const { Q_D(const SevenSegmentDisplay); return d->mDisplayNode->getHAlignment(); }
void SevenSegmentDisplay::setHorizontalAlignment(Alignment alignment)
{
	Q_D(SevenSegmentDisplay);
	if(d->mDisplayNode->setHAlignment(alignment))
	{
		update();
		emit horizontalAlignmentChanged();
	}
}

QColor SevenSegmentDisplay::getBgColor() const { Q_D(const SevenSegmentDisplay); return d->mDisplayNode->getBgColor(); }
void SevenSegmentDisplay::setBgColor(const QColor& color)
{
	Q_D(SevenSegmentDisplay);
	if(d->mDisplayNode->setBgColor(color))
	{
		update();
		emit bgColorChanged();
	}
}

QColor SevenSegmentDisplay::getOnColor() const { Q_D(const SevenSegmentDisplay); return d->mDisplayNode->getOnColor(); }
void SevenSegmentDisplay::setOnColor(const QColor& color)
{
	Q_D(SevenSegmentDisplay);
	if(d->mDisplayNode->setOnColor(color))
	{
		update();
		emit onColorChanged();
	}
}

QColor SevenSegmentDisplay::getOffColor() const { Q_D(const SevenSegmentDisplay); return d->mDisplayNode->getOffColor(); }
void SevenSegmentDisplay::setOffColor(const QColor& color)
{
	Q_D(SevenSegmentDisplay);
	if(d->mDisplayNode->setOffColor(color))
	{
		update();
		emit offColorChanged();
	}
}

/** \internal Called on render thread when update is needed. */
QSGNode* SevenSegmentDisplay::updatePaintNode(QSGNode* oldRoot, QQuickItem::UpdatePaintNodeData* /*d*/)
{
	Q_D(SevenSegmentDisplay);
	DisplayNode* displayNode = static_cast<DisplayNode*>(oldRoot);
	if (!displayNode)
	{
		displayNode = d->mDisplayNode;
		connect(displayNode, &DisplayNode::overflow, this, &SevenSegmentDisplay::overflow);
	}

	// Update digit and all its children
	QSizeF contentSize = displayNode->update(boundingRect());
#if 0
	qDebug() << "boundingRect:" << boundingRect();
	qDebug() << "contentSize:" << contentSize;
#endif

	// Use the returned size as implicit item sizes (imitate behavior of QQuicks Text item).
	setImplicitWidth(contentSize.width());
	setImplicitHeight(contentSize.height());

	return displayNode;
}
