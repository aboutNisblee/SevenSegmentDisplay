/**
 * \file sevensegmentdisplay.cpp
 *
 * \date 04.09.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include <gui/sevensegmentdisplay.hpp>

#include <memory>
#include <algorithm>
#include <QSGGeometryNode>
#include <QSGSimpleRectNode>
#include <QSGFlatColorMaterial>
#include <QMatrix>

/* Structure that is used to share settings between GUI and Render-Thread. */
struct DigitNodeSettings
{
	int mValue = 0;
	int mDigitSize = 24;
	bool dot = false;
	SevenSegmentDisplay::Alignment mVAlignment = SevenSegmentDisplay::AlignCenter;
	SevenSegmentDisplay::Alignment mHAlignment = SevenSegmentDisplay::AlignCenter;
	QColor mBgColor = QColor("black");
	QColor mOnColor = QColor("green");
	QColor mOffColor = QColor("gray");
};

namespace
{

// XXX: Configurable??
/* Sizes in un-scaled coordinate system. */
constexpr qreal baseSegLength = 2.0;
constexpr qreal baseSegWidth = 0.60;
constexpr qreal baseSegGap = 0.45 / 10;
constexpr qreal baseDigitHeight = 2 * baseSegLength + baseSegWidth + 4 * baseSegGap;

constexpr qreal baseDotRadius = baseSegWidth * 0.6;
constexpr quint8 dotSegs = 24;

template<int count>
class ElementNode: public QSGGeometryNode
{
public:
	virtual ~ElementNode() {}
	/** \internal Rotate the element. */
	inline ElementNode& rotate(qreal deg)
	{
		QMatrix m = QMatrix().rotate(deg);
		for (QPointF& v : mV)
			v = m.map(v);
		return *this;
	}
	inline QPointF getEffectiveVertex(qint8 no)
	{
		if (no < mGeo->vertexCount())
			return QPointF(mGeo->vertexDataAsPoint2D()[no].x, mGeo->vertexDataAsPoint2D()[no].y);
		return QPointF();
	}
	/** \internal Update the foreground color of the element. */
	inline void setColor(const QColor& color)
	{
		if (mMat->color() != color)
		{
			mMat->setColor(color);
			markDirty(QSGNode::DirtyMaterial);
		}
	}
	virtual void updateGeometry(const QMatrix& trans) = 0;
protected:
	std::array<QPointF, count> mV;
	std::unique_ptr<QSGGeometry> mGeo;
	std::unique_ptr<QSGFlatColorMaterial> mMat;
};

/** \internal Scene graph geometry node of a single segment. */
struct SegmentNode: public ElementNode<6>
{
	SegmentNode()
	{
		mGeo = std::unique_ptr<QSGGeometry>(new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), mV.size()));
		mGeo->setDrawingMode(GL_TRIANGLE_STRIP);
		setGeometry(mGeo.get());
		mMat = std::unique_ptr<QSGFlatColorMaterial>(new QSGFlatColorMaterial);
		setMaterial(mMat.get());
		reset();
	}
	/** \internal Reset the segment to its initial state. */
	inline SegmentNode& reset()
	{
		/* Vertices are placed in the center of a 2-dimensional coordinate system to simplify initial rotation.
		 * The y values are increasing downwards to ease mapping to Quicks coordinate system.
		 *   /v2---------v4\
		 * v0               v5
		 *   \v1---------v3/
		 * */
		mV[0] = { -baseSegLength / 2, 0};
		mV[1] = { -baseSegLength / 2 + baseSegWidth / 2, baseSegWidth / 2};
		mV[2] = { -baseSegLength / 2 + baseSegWidth / 2, -baseSegWidth / 2};
		mV[3] = {baseSegLength / 2 - baseSegWidth / 2, baseSegWidth / 2};
		mV[4] = {baseSegLength / 2 - baseSegWidth / 2, -baseSegWidth / 2};
		mV[5] = {baseSegLength / 2, 0};
		return *this;
	}
	/** \internal Update the geometry by mapping the current segment into the coordinate system of the given matrix. */
	inline void updateGeometry(const QMatrix& trans)
	{
		bool dirty = false;
		QSGGeometry::Point2D* data = mGeo->vertexDataAsPoint2D();
		for (int i = 0; i < mGeo->vertexCount(); ++i)
		{
			// QPointF is is actually QPointDouble! So take care in comparisons!
			QPointF p = trans.map(mV[i]);
			float pX = static_cast<float>(p.x());
			float pY = static_cast<float>(p.y());
			if (pX != data[i].x || pY != data[i].y)
			{
				data[i].set(pX, pY);
				dirty = true;
			}
		}

		if (dirty)
			markDirty(QSGNode::DirtyGeometry);
	}
};

struct DotNode: public ElementNode < dotSegs + 2 >
{
	DotNode()
	{
		mGeo = std::unique_ptr<QSGGeometry>(new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), mV.size()));
		mGeo->setDrawingMode(GL_TRIANGLE_FAN);
		setGeometry(mGeo.get());
		mMat = std::unique_ptr<QSGFlatColorMaterial>(new QSGFlatColorMaterial);
		setMaterial(mMat.get());
		reset();
	}
	/** \internal Reset the dot to its initial state. */
	inline DotNode& reset()
	{
		mV[0] = { 0, 0};
		mV[1] = { baseDotRadius, 0};
		QMatrix m = QMatrix().rotate(360 / dotSegs);
		for (int i = 2; i < dotSegs + 2; ++i)
			mV[i] = m.map(mV[i - 1]);
		return *this;
	}
	/** \internal Update the geometry by mapping the current segment into the coordinate system of the given matrix. */
	inline void updateGeometry(const QMatrix& trans)
	{
		bool dirty = false;
		QSGGeometry::Point2D* data = mGeo->vertexDataAsPoint2D();
		for (int i = 0; i < mGeo->vertexCount(); ++i)
		{
			// QPointF is is actually QPointDouble! So take care in comparisons!
			QPointF p = trans.map(mV[i]);
			float pX = static_cast<float>(p.x());
			float pY = static_cast<float>(p.y());
			if (pX != data[i].x || pY != data[i].y)
			{
				data[i].set(pX, pY);
				dirty = true;
				//				qDebug() << "Dirty!";
			}
		}

		if (dirty)
			markDirty(QSGNode::DirtyGeometry);
	}
};

/** \internal Scene graph node of a digit (7 segments + dot and background color). */
class DigitNode: public QSGSimpleRectNode
{
public:

	/* TODO: Use a config structure defined by this class and pass it
	 * in as shared_ptr from client. Pass the whole or only a part to all
	 * segments. -> No need for coping the values to all segments nor digits.
	 */

	/** \internal Construct a new digit.
	 * \note The node is typically owned by the render thread. To easily share the
	 * property values of the client with the node in the render thread, a DigitNodeSettings object is used.
	 * Synchronization inst needed since the GUI thread is locked while the the digit node accesses the settings object.
	 */
	explicit DigitNode(std::shared_ptr<DigitNodeSettings> settings)
	{
		mSettings = settings;
		if (!mSettings)
			mSettings = std::make_shared<DigitNodeSettings>(DigitNodeSettings());

		mSegments[0] = std::unique_ptr<SegmentNode>(new SegmentNode);
		appendChildNode(mSegments[0].get());
		mSegments[1] = std::unique_ptr<SegmentNode>(new SegmentNode);
		mSegments[1]->rotate(90);
		appendChildNode(mSegments[1].get());
		mSegments[2] = std::unique_ptr<SegmentNode>(new SegmentNode);
		mSegments[2]->rotate(90);
		appendChildNode(mSegments[2].get());
		mSegments[3] = std::unique_ptr<SegmentNode>(new SegmentNode);
		appendChildNode(mSegments[3].get());
		mSegments[4] = std::unique_ptr<SegmentNode>(new SegmentNode);
		mSegments[4]->rotate(90);
		appendChildNode(mSegments[4].get());
		mSegments[5] = std::unique_ptr<SegmentNode>(new SegmentNode);
		mSegments[5]->rotate(90);
		appendChildNode(mSegments[5].get());
		mSegments[6] = std::unique_ptr<SegmentNode>(new SegmentNode);
		appendChildNode(mSegments[6].get());

		mDot = std::unique_ptr<DotNode>(new DotNode);
		appendChildNode(mDot.get());
	}

	/** \internal Update the state of the node and all its children.
	 * Things are only updated when the settings differ from last call.
	 * @param rectangle The current binding rectangle of the caller.
	 */
	QRectF update(const QRectF& bound)
	{
		QRectF resultingRect = bound;

		// FIXME: Not optimal! Some of the values aren't needed to recalculate on each update!

		// Calculate needed scale to match requested digit size
		qreal scale = mSettings->mDigitSize / baseDigitHeight;
		// Scale all segment sizes, needed for positioning
		qreal segWidth = baseSegWidth * scale;
		qreal segLength = baseSegLength * scale;
		qreal segGap = baseSegGap * scale;
		qreal dotRadius = baseDotRadius * scale;
		// Create a scale matrix, needed to scale basic vertices
		QMatrix matScale = QMatrix().scale(scale, scale);

		// If layout isn't specified by client, set it!
		if (resultingRect.width() <= 0)
			resultingRect.setWidth((segLength + segWidth + 2 * segGap) * 1.4); // TODO: Add LRMargins
		if (resultingRect.height() <= 0)
			resultingRect.setHeight(mSettings->mDigitSize * 1.4); // TODO: Add TBMargins

		// Update rectangle of the background
		if (rect() != resultingRect)
		{
			setRect(resultingRect);
			markDirty(QSGNode::DirtyGeometry);
		}

		qreal digitCenterX = 0;
		qreal digitCenterY = 0;

		switch (mSettings->mHAlignment)
		{
		case SevenSegmentDisplay::AlignCenter:
			digitCenterX = resultingRect.center().x();
			break;
		}

		switch (mSettings->mVAlignment)
		{
		case SevenSegmentDisplay::AlignCenter:
			digitCenterY = resultingRect.center().y();
			break;
		}

		// (A) top
		mSegments[0]->updateGeometry(matScale * QMatrix().translate(digitCenterX, digitCenterY - segLength - segGap * 2));
		// (B) top right
		mSegments[1]->updateGeometry(matScale * QMatrix().translate(digitCenterX + segLength / 2 + segGap,
		                             digitCenterY - segLength / 2 - segGap));
		// (C) bottom right
		mSegments[2]->updateGeometry(matScale * QMatrix().translate(digitCenterX + segLength / 2 + segGap,
		                             digitCenterY + segLength / 2 + segGap));
		// (D) bottom
		mSegments[3]->updateGeometry(matScale * QMatrix().translate(digitCenterX, digitCenterY + segLength + segGap * 2));
		// (E) bottom left
		mSegments[4]->updateGeometry(matScale * QMatrix().translate(digitCenterX - segLength / 2 - segGap,
		                             digitCenterY + segLength / 2 + segGap));
		// (F) top left
		mSegments[5]->updateGeometry(matScale * QMatrix().translate(digitCenterX - segLength / 2 - segGap,
		                             digitCenterY - segLength / 2 - segGap));
		// (G) middle
		mSegments[6]->updateGeometry(matScale * QMatrix().translate(digitCenterX, digitCenterY));

		// XXX: DEBUGGING
		mDot->updateGeometry(matScale * QMatrix().translate(digitCenterX + segLength / 2 + segGap + segWidth / 2 + dotRadius +
		                     segGap,
		                     digitCenterY + segLength + 2 * segGap + segWidth / 2 - dotRadius));

#if 0
		qDebug() << "Effective digit size:" << mSegments[3]->getEffectiveVertex(1).y() - mSegments[0]->getEffectiveVertex(
		             2).y();
#endif

		/* Background color
		 * This will only mark the material dirty when color differs from current one. */
		setColor(mSettings->mBgColor);

		/* Value
		 * Segment colors are updated, because each segment is touched.
		 * Segment material is only marked dirty, when color/value is changed. */
		if (mSettings->mValue >= 0 && mSettings->mValue < 10)
		{
			quint8 code = codeLut[mSettings->mValue];
			quint8 mask = 0x01;
			for (quint8 i = 0; i < 7; ++i)
			{
				if (code & mask)
					mSegments[i]->setColor(mSettings->mOnColor);
				else
					mSegments[i]->setColor(mSettings->mOffColor);
				mask = mask << 1;
			}
		}
		else
		{
			qWarning() << "Invalid value" << mSettings->mValue;
		}

		mDot->setColor(mSettings->mOnColor);

		return resultingRect;
	}

private:
	std::array<std::unique_ptr<SegmentNode>, 7> mSegments;
	std::unique_ptr<DotNode> mDot;
	std::shared_ptr<DigitNodeSettings> mSettings;

	/* 0 0×3F, 1 0×06, 2 0×5B, 3 0×4F, 4 0×66, 5 0×6D, 6 0×7D, 7 0×07, 8 0×7F, 9 0×6F */
	static const quint8 codeLut[10];
};

const quint8 DigitNode::codeLut[] =
{ 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f };

} // namespace

SevenSegmentDisplay::SevenSegmentDisplay(QQuickItem* parent) :
	QQuickItem(parent)
{
	mDigitSettings = std::make_shared<DigitNodeSettings>(DigitNodeSettings());
	mDigitSettings->mOnColor = QColor("red");
	mDigitSettings->mOffColor = QColor("black");

	setFlag(ItemHasContents, true);

	//	connect(this, &QQuickItem::widthChanged, this, [&]() { qDebug() << "width:" << width(); });
	//	connect(this, &QQuickItem::heightChanged, this, [&]() { qDebug() << "height:" << height(); });
}

int SevenSegmentDisplay::getValue() const { return mDigitSettings->mValue; }
void SevenSegmentDisplay::setValue(int value)
{
	if (mDigitSettings->mValue != value)
	{
		mDigitSettings->mValue = value;
		update();
		emit valueChanged();
	}
}

int SevenSegmentDisplay::getDigitSize() const { return mDigitSettings->mDigitSize; }
void SevenSegmentDisplay::setDigitSize(int size)
{
	if (mDigitSettings->mDigitSize != size)
	{
		mDigitSettings->mDigitSize = size;
		update();
		emit digitSizeChanged();
	}
}

SevenSegmentDisplay::Alignment SevenSegmentDisplay::getVerticalAlignment() const { return mDigitSettings->mVAlignment; }
void SevenSegmentDisplay::setVerticalAlignment(Alignment alignment)
{
	if (mDigitSettings->mVAlignment != alignment)
	{
		mDigitSettings->mVAlignment = alignment;
		update();
		emit verticalAlignmentChanged();
	}
}

SevenSegmentDisplay::Alignment SevenSegmentDisplay::getHorizontalAlignment() const { return mDigitSettings->mHAlignment; }
void SevenSegmentDisplay::setHorizontalAlignment(Alignment alignment)
{
	if (mDigitSettings->mHAlignment != alignment)
	{
		mDigitSettings->mHAlignment = alignment;
		update();
		emit horizontalAlignmentChanged();
	}
}

QColor SevenSegmentDisplay::getBgColor() const { return mDigitSettings->mBgColor; }
void SevenSegmentDisplay::setBgColor(const QColor& color)
{
	if (mDigitSettings->mBgColor != color)
	{
		mDigitSettings->mBgColor = color;
		update();
		emit bgColorChanged();
	}
}

QColor SevenSegmentDisplay::getOnColor() const { return mDigitSettings->mOnColor; }
void SevenSegmentDisplay::setOnColor(const QColor& color)
{
	if (mDigitSettings->mOnColor != color)
	{
		mDigitSettings->mOnColor = color;
		update();
		emit onColorChanged();
	}
}

QColor SevenSegmentDisplay::getOffColor() const { return mDigitSettings->mOffColor; }
void SevenSegmentDisplay::setOffColor(const QColor& color)
{
	if (mDigitSettings->mOffColor != color)
	{
		mDigitSettings->mOffColor = color;
		update();
		emit offColorChanged();
	}
}

/** \internal Called on render thread when update is needed. */
QSGNode* SevenSegmentDisplay::updatePaintNode(QSGNode* oldRoot, QQuickItem::UpdatePaintNodeData* /*d*/)
{
	DigitNode* digit = static_cast<DigitNode*>(oldRoot);
	if (!digit)
	{
		digit = new DigitNode(mDigitSettings);
	}

	// Update digit and all its children, based on values in mDigitSettings.
	QRectF resultingRect = digit->update(boundingRect());

	// Use the returned size as implicit item sizes (imitate behavior of QQuicks Text item).
	setImplicitWidth(resultingRect.width());
	setImplicitHeight(resultingRect.height());

	return digit;
}
