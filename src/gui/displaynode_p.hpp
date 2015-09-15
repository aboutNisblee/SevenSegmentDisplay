/**
 * \file displaynode_p.hpp
 *
 * \date 14.09.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef DISPLAYNODE_P_HPP_
#define DISPLAYNODE_P_HPP_

#include <memory>
#include <algorithm>
#include <vector>

#include <QSGGeometryNode>
#include <QSGSimpleRectNode>
#include <QSGFlatColorMaterial>
#include <QMatrix>

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

/* 0 0×3F, 1 0×06, 2 0×5B, 3 0×4F, 4 0×66, 5 0×6D, 6 0×7D, 7 0×07, 8 0×7F, 9 0×6F */
constexpr quint8 lutSegCode[] =
{ 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f };
} // namespace

template<int count>
class ElementNode: public QSGGeometryNode
{
public:
	virtual ~ElementNode() {}
	/** \internal Rotate the element. */
	inline ElementNode& rotate(qreal deg)
	{
		QMatrix m = QMatrix().rotate(deg);
		for (QPointF& v : mVertices)
			v = m.map(v);
		return *this;
	}
	inline QPointF getEffectiveVertex(qint8 no)
	{
		if (no < mGeometry->vertexCount())
			return QPointF(mGeometry->vertexDataAsPoint2D()[no].x, mGeometry->vertexDataAsPoint2D()[no].y);
		return QPointF();
	}
	/** \internal Update the foreground color of the element. */
	inline void setColor(const QColor& color)
	{
		if (mMaterial->color() != color)
		{
			mMaterial->setColor(color);
			markDirty(QSGNode::DirtyMaterial);
		}
	}
	virtual void updateGeometry(const QMatrix& trans) = 0;
protected:
	std::array<QPointF, count> mVertices;
	std::unique_ptr<QSGGeometry> mGeometry;
	std::unique_ptr<QSGFlatColorMaterial> mMaterial;
};

/** \internal Scene graph geometry node of a single segment. */
struct SegmentNode: public ElementNode<6>
{
	SegmentNode()
	{
		mGeometry = std::unique_ptr<QSGGeometry>(new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), mVertices.size()));
		mGeometry->setDrawingMode(GL_TRIANGLE_STRIP);
		setGeometry(mGeometry.get());
		mMaterial = std::unique_ptr<QSGFlatColorMaterial>(new QSGFlatColorMaterial);
		setMaterial(mMaterial.get());
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
		mVertices[0] = { -baseSegLength / 2, 0};
		mVertices[1] = { -baseSegLength / 2 + baseSegWidth / 2, baseSegWidth / 2};
		mVertices[2] = { -baseSegLength / 2 + baseSegWidth / 2, -baseSegWidth / 2};
		mVertices[3] = {baseSegLength / 2 - baseSegWidth / 2, baseSegWidth / 2};
		mVertices[4] = {baseSegLength / 2 - baseSegWidth / 2, -baseSegWidth / 2};
		mVertices[5] = {baseSegLength / 2, 0};
		return *this;
	}
	/** \internal Update the geometry by mapping the current segment into the coordinate system of the given matrix. */
	inline void updateGeometry(const QMatrix& trans)
	{
		bool dirty = false;
		QSGGeometry::Point2D* data = mGeometry->vertexDataAsPoint2D();
		for (int i = 0; i < mGeometry->vertexCount(); ++i)
		{
			// QPointF is is actually QPointDouble! So take care in comparisons!
			QPointF p = trans.map(mVertices[i]);
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
		mGeometry = std::unique_ptr<QSGGeometry>(new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), mVertices.size()));
		mGeometry->setDrawingMode(GL_TRIANGLE_FAN);
		setGeometry(mGeometry.get());
		mMaterial = std::unique_ptr<QSGFlatColorMaterial>(new QSGFlatColorMaterial);
		setMaterial(mMaterial.get());
		reset();
	}
	/** \internal Reset the dot to its initial state. */
	inline DotNode& reset()
	{
		mVertices[0] = { 0, 0};
		mVertices[1] = { baseDotRadius, 0};
		QMatrix m = QMatrix().rotate(360 / dotSegs);
		for (int i = 2; i < dotSegs + 2; ++i)
			mVertices[i] = m.map(mVertices[i - 1]);
		return *this;
	}
	/** \internal Update the geometry by mapping the current segment into the coordinate system of the given matrix. */
	inline void updateGeometry(const QMatrix& trans)
	{
		bool dirty = false;
		QSGGeometry::Point2D* data = mGeometry->vertexDataAsPoint2D();
		for (int i = 0; i < mGeometry->vertexCount(); ++i)
		{
			// QPointF is is actually QPointDouble! So take care in comparisons!
			QPointF p = trans.map(mVertices[i]);
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

/** \internal Scene graph node of a digit (7 segments + dot). */
struct DigitNode: public QSGSimpleRectNode
{
	/* TODO: Use a config structure defined by this class and pass it
	 * in as shared_ptr from client. Pass the whole or only a part to all
	 * segments. -> No need for coping the values to all segments nor digits.
	 */

	/** \internal Construct a new digit.
	 * \note The node is typically owned by the render thread. To easily share the
	 * property values of the client with the node in the render thread, a DigitNodeSettings object is used.
	 * Synchronization inst needed since the GUI thread is locked while the the digit node accesses the settings object.
	 */
	DigitNode()
	{
		SegmentNode* segment = new SegmentNode;
		appendChildNode(segment);

		segment = new SegmentNode;
		segment->rotate(90);
		appendChildNode(segment);

		segment = new SegmentNode;
		segment->rotate(90);
		appendChildNode(segment);

		segment = new SegmentNode;
		appendChildNode(segment);

		segment = new SegmentNode;
		segment->rotate(90);
		appendChildNode(segment);

		segment = new SegmentNode;
		segment->rotate(90);
		appendChildNode(segment);

		segment = new SegmentNode;
		appendChildNode(segment);

		DotNode* dot = new DotNode;
		appendChildNode(dot);
	}

	/** \internal Update the state of the node and all its children.
	 * Things are only updated when the settings differ from last call.
	 * @param rectangle The current binding rectangle of the caller.
	 */
	inline void update(qreal scale)
	{
		// Scale all segment sizes, needed for positioning
		qreal segWidth = baseSegWidth * scale;
		qreal segLength = baseSegLength * scale;
		qreal segGap = baseSegGap * scale;
		qreal dotRadius = baseDotRadius * scale;
		// Create a scale matrix, needed to scale basic vertices
		QMatrix matScale = QMatrix().scale(scale, scale);

		qreal digitCenterX = rect().center().x() - (2 * dotRadius + segGap) / 2;
		qreal digitCenterY = rect().center().y();

		Q_ASSERT(8 == childCount());

		// (A) top
		static_cast<SegmentNode*>(childAtIndex(0))->updateGeometry(matScale * QMatrix().translate(digitCenterX,
		        digitCenterY - segLength - segGap * 2));
		// (B) top right
		static_cast<SegmentNode*>(childAtIndex(1))->updateGeometry(matScale * QMatrix().translate(
		            digitCenterX + segLength / 2 + segGap,
		            digitCenterY - segLength / 2 - segGap));
		// (C) bottom right
		static_cast<SegmentNode*>(childAtIndex(2))->updateGeometry(matScale * QMatrix().translate(
		            digitCenterX + segLength / 2 + segGap,
		            digitCenterY + segLength / 2 + segGap));
		// (D) bottom
		static_cast<SegmentNode*>(childAtIndex(3))->updateGeometry(matScale * QMatrix().translate(digitCenterX,
		        digitCenterY + segLength + segGap * 2));
		// (E) bottom left
		static_cast<SegmentNode*>(childAtIndex(4))->updateGeometry(matScale * QMatrix().translate(
		            digitCenterX - segLength / 2 - segGap,
		            digitCenterY + segLength / 2 + segGap));
		// (F) top left
		static_cast<SegmentNode*>(childAtIndex(5))->updateGeometry(matScale * QMatrix().translate(
		            digitCenterX - segLength / 2 - segGap,
		            digitCenterY - segLength / 2 - segGap));
		// (G) middle
		static_cast<SegmentNode*>(childAtIndex(6))->updateGeometry(matScale * QMatrix().translate(digitCenterX,
		        digitCenterY));

		// The dot is always laid out. Appearance is controlled by color.
		static_cast<DotNode*>(childAtIndex(7))->updateGeometry(matScale * QMatrix().translate(
		            digitCenterX + segLength / 2 + segGap + segWidth / 2 + dotRadius +
		            segGap,
		            digitCenterY + segLength + 2 * segGap + segWidth / 2 - dotRadius));

#if 0
		qDebug() << "Effective digit size:" << static_cast<SegmentNode*>(childAtIndex(3))->getEffectiveVertex(
		             1).y() - static_cast<SegmentNode*>(childAtIndex(0))->getEffectiveVertex(
		             2).y();
#endif
	}

	inline void setValue(int value, QColor onColor, QColor offColor)
	{
		Q_ASSERT(8 == childCount());

		if (value >= 0 && value < 10)
		{
			quint8 code = lutSegCode[value];
			quint8 mask = 0x01;
			for (quint8 i = 0; i < 7; ++i)
			{
				/* Segment material is only marked dirty, when color is changed. */
				static_cast<SegmentNode*>(childAtIndex(i))->setColor((code & mask) ? onColor : offColor);
				mask = mask << 1;
			}
		}
		else
		{
			qWarning() << "Invalid value" << value;
		}

		// TODO!
		static_cast<DotNode*>(lastChild())->setColor(onColor);
	}

	static constexpr qreal width()
	{
		return baseSegLength  + baseSegWidth  + 2 * baseSegGap  + 2 * baseDotRadius  + baseSegGap ;
	}
};

/** \internal Root scene graph node if the display. */
class DisplayNode: public QSGSimpleRectNode
{
public:
	int getDigitCount() const { return mDigitCount; }
	bool setDigitCount(int digitCount)
	{
		if (digitCount == mDigitCount)
			return false;
		mDigitCount = digitCount;
		return true;
	}

	int getValue() const { return mValue; }
	bool setValue(int value)
	{
		if (value == mValue)
			return false;
		mValue = value;
		return true;
	}

	int getDigitSize() const { return mDigitSize; }
	bool setDigitSize(int digitSize)
	{
		if (digitSize == mDigitSize)
			return false;
		mDigitSize = digitSize;
		return true;
	}

	SevenSegmentDisplay::Alignment getHAlignment() const { return mHAlignment; }
	bool setHAlignment(SevenSegmentDisplay::Alignment hAlignment)
	{
		if (hAlignment == mHAlignment)
			return false;
		mHAlignment = hAlignment;
		return true;
	}

	SevenSegmentDisplay::Alignment getVAlignment() const { return mVAlignment; }
	bool setVAlignment(SevenSegmentDisplay::Alignment vAlignment)
	{
		if (vAlignment == mVAlignment)
			return false;
		mVAlignment = vAlignment;
		return true;
	}

	const QColor& getBgColor() const { return mBgColor; }
	bool setBgColor(const QColor& bgColor)
	{
		if (bgColor == mBgColor)
			return false;
		mBgColor = bgColor;
		return true;
	}

	const QColor& getOnColor() const { return mOnColor; }
	bool setOnColor(const QColor& onColor)
	{
		if (onColor == mOnColor)
			return false;
		mOnColor = onColor;
		return true;
	}

	const QColor& getOffColor() const { return mOffColor; }
	bool setOffColor(const QColor& offColor)
	{
		if (offColor == mOffColor)
			return false;
		mOffColor = offColor;
		return true;
	}

	inline QSizeF update(const QRectF& boundingRectange)
	{
		QRectF contentRect;

		// Digit count
		while (childCount() != mDigitCount)
		{
			if (childCount() < mDigitCount)
			{
				qDebug() << "Adding new digit";
				appendChildNode(new DigitNode);
			}
			else
			{
				qDebug() << "Removing digit";
				removeChildNode(lastChild());
			}
		}

		if (0 == childCount())
			return contentRect.size();

		// Calculate needed scale to match requested digit size
		qreal scale = mDigitSize / baseDigitHeight;

		// Calculate content size
		contentRect.setWidth(DigitNode::width() * scale * mDigitCount);
		contentRect.setHeight(mDigitSize);

		// Update rectangle of the background to the maximum of the size of the given rectangle and the content size
		if (rect().size() != contentRect.size().expandedTo(boundingRectange.size()))
		{
			setRect(QRectF(QPointF(), contentRect.size().expandedTo(boundingRectange.size())));
			markDirty(QSGNode::DirtyGeometry);
		}

		// Move the content rectangle to top left of the bounding rectangle
		contentRect.moveTopLeft(rect().topLeft());

		/* Set the background color
		 * This will only mark the material dirty when color differs from current one. */
		setColor(mBgColor);

		// Horizontal alignment
		if (contentRect.width() < rect().width())
		{
			switch (mHAlignment)
			{
			case SevenSegmentDisplay::AlignLeft:
				break;
			case SevenSegmentDisplay::AlignTop:
				qDebug() << "Incompatible alignment: AlignTop as horizontal alignment";
				break;
			case SevenSegmentDisplay::AlignCenter:
				contentRect.moveCenter(QPointF(rect().center().x(), contentRect.center().y()));
				break;
			}
		}

		// Verical alignment
		if (contentRect.height() < rect().height())
		{
			switch (mVAlignment)
			{
			case SevenSegmentDisplay::AlignLeft:
				qDebug() << "Incompatible alignment: AlignLeft as vertical alignment";
				break;
			case SevenSegmentDisplay::AlignTop:
				break;
			case SevenSegmentDisplay::AlignCenter:
				contentRect.moveCenter(QPointF(contentRect.center().x(), rect().center().y()));
				break;
			}
		}

		// Split the content pane into digit parts
		QRectF digitRect = contentRect;
		digitRect.setWidth(digitRect.width() / mDigitCount);

		// Update digits
		Q_ASSERT(childCount() == mDigitCount);
		for (int i = 0; i < mDigitCount; ++i)
		{
			DigitNode* digit = static_cast<DigitNode*>(childAtIndex(i));

			// Move digit rect to the right position
			QRectF rect = digitRect;
			rect.moveLeft(rect.left() + rect.width() * i);
			digit->setRect(rect);

			digit->update(scale);

			digit->setValue(mValue, mOnColor, mOffColor);

			digit->setColor(Qt::transparent);
			//			digit->setColor("gray");
		}

		return contentRect.size();
	}

private:
	int mDigitCount = 1;
	int mValue = 0;
	int mDigitSize = 24;
	SevenSegmentDisplay::Alignment mHAlignment = SevenSegmentDisplay::AlignLeft;
	SevenSegmentDisplay::Alignment mVAlignment = SevenSegmentDisplay::AlignTop;
	QColor mBgColor = QColor("black");
	QColor mOnColor = QColor("green");
	QColor mOffColor = QColor("gray");
};

#endif /* DISPLAYNODE_P_HPP_ */
