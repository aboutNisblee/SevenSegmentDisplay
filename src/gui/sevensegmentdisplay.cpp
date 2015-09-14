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
	SevenSegmentDisplayPrivate()
	{
		mDisplayState = std::make_shared<DisplayState>(DisplayState());
	}
	Q_DISABLE_COPY(SevenSegmentDisplayPrivate)

	std::shared_ptr<DisplayState> mDisplayState;

	// Owned by scene graph
	//	DisplayNode* mDisplayNode = nullptr;
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

int SevenSegmentDisplay::getDigitCount() const { Q_D(const SevenSegmentDisplay); return d->mDisplayState->mDigitCount; }
void SevenSegmentDisplay::setDigitCount(int count)
{
	Q_D(SevenSegmentDisplay);
	if (d->mDisplayState->mDigitCount != count)
	{
		d->mDisplayState->mDigitCount = count;
		update();
		emit digitCountChanged();
	}
}

int SevenSegmentDisplay::getValue() const { Q_D(const SevenSegmentDisplay); return d->mDisplayState->mValue; }
void SevenSegmentDisplay::setValue(int value)
{
	Q_D(SevenSegmentDisplay);
	if (d->mDisplayState->mValue != value)
	{
		d->mDisplayState->mValue = value;
		update();
		emit valueChanged();
	}
}

int SevenSegmentDisplay::getDigitSize() const { Q_D(const SevenSegmentDisplay); return d->mDisplayState->mDigitSize; }
void SevenSegmentDisplay::setDigitSize(int size)
{
	Q_D(SevenSegmentDisplay);
	if (d->mDisplayState->mDigitSize != size)
	{
		d->mDisplayState->mDigitSize = size;
		update();
		emit digitSizeChanged();
	}
}

SevenSegmentDisplay::Alignment SevenSegmentDisplay::getVerticalAlignment() const { Q_D(const SevenSegmentDisplay); return d->mDisplayState->mVAlignment; }
void SevenSegmentDisplay::setVerticalAlignment(Alignment alignment)
{
	Q_D(SevenSegmentDisplay);
	if (d->mDisplayState->mVAlignment != alignment)
	{
		d->mDisplayState->mVAlignment = alignment;
		update();
		emit verticalAlignmentChanged();
	}
}

SevenSegmentDisplay::Alignment SevenSegmentDisplay::getHorizontalAlignment() const { Q_D(const SevenSegmentDisplay); return d->mDisplayState->mHAlignment; }
void SevenSegmentDisplay::setHorizontalAlignment(Alignment alignment)
{
	Q_D(SevenSegmentDisplay);
	if (d->mDisplayState->mHAlignment != alignment)
	{
		d->mDisplayState->mHAlignment = alignment;
		update();
		emit horizontalAlignmentChanged();
	}
}

QColor SevenSegmentDisplay::getBgColor() const { Q_D(const SevenSegmentDisplay); return d->mDisplayState->mBgColor; }
void SevenSegmentDisplay::setBgColor(const QColor& color)
{
	Q_D(SevenSegmentDisplay);
	if (d->mDisplayState->mBgColor != color)
	{
		d->mDisplayState->mBgColor = color;
		update();
		emit bgColorChanged();
	}
}

QColor SevenSegmentDisplay::getOnColor() const { Q_D(const SevenSegmentDisplay); return d->mDisplayState->mOnColor; }
void SevenSegmentDisplay::setOnColor(const QColor& color)
{
	Q_D(SevenSegmentDisplay);
	if (d->mDisplayState->mOnColor != color)
	{
		d->mDisplayState->mOnColor = color;
		update();
		emit onColorChanged();
	}
}

QColor SevenSegmentDisplay::getOffColor() const { Q_D(const SevenSegmentDisplay); return d->mDisplayState->mOffColor; }
void SevenSegmentDisplay::setOffColor(const QColor& color)
{
	Q_D(SevenSegmentDisplay);
	if (d->mDisplayState->mOffColor != color)
	{
		d->mDisplayState->mOffColor = color;
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
		displayNode = new DisplayNode(d->mDisplayState);
	}

	// Update digit and all its children, based on values in mDisplayState.
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
