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
#include <QMatrix>

namespace
{
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
	inline QPointF getEffectiveVertex(qint8 no)
	{
		if (no < mGeometry->vertexCount())
			return QPointF(mGeometry->vertexDataAsPoint2D()[no].x, mGeometry->vertexDataAsPoint2D()[no].y);
		return QPointF();
	}
	/** \internal Update the foreground color of the element. */
	inline void setColor(const QColor& color)
	{
		if (mMaterial->color() == color)
			return;

		mMaterial->setColor(color);
		markDirty(QSGNode::DirtyMaterial);
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
	explicit SegmentNode(qreal deg = 0)
	{
		mGeometry = std::unique_ptr<QSGGeometry>(new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), mVertices.size()));
		mGeometry->setDrawingMode(GL_TRIANGLE_STRIP);
		setGeometry(mGeometry.get());
		mMaterial = std::unique_ptr<QSGFlatColorMaterial>(new QSGFlatColorMaterial);
		setMaterial(mMaterial.get());

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

		if (deg)
		{
			QMatrix m = QMatrix().rotate(deg);
			for (QPointF& v : mVertices)
				v = m.map(v);
		}
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

		mVertices[0] = { 0, 0};
		mVertices[1] = { baseDotRadius, 0};
		QMatrix m = QMatrix().rotate(360 / dotSegs);
		for (int i = 2; i < dotSegs + 2; ++i)
			mVertices[i] = m.map(mVertices[i - 1]);
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

/** \internal Scene graph node of a digit (7 segments + dot). */
struct DigitNode: public QSGSimpleRectNode
{
	/** \internal Construct a new digit.
	 * \note The node is typically owned by the render thread. To easily share the
	 * property values of the client with the node in the render thread, a DigitNodeSettings object is used.
	 * Synchronization inst needed since the GUI thread is locked while the the digit node accesses the settings object.
	 */
	DigitNode()
	{
		appendChildNode(new SegmentNode());
		appendChildNode(new SegmentNode(90));
		appendChildNode(new SegmentNode(90));
		appendChildNode(new SegmentNode());
		appendChildNode(new SegmentNode(90));
		appendChildNode(new SegmentNode(90));
		appendChildNode(new SegmentNode());
		appendChildNode(new DotNode);

		setColor(Qt::transparent);
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
	inline int getDigitCount() const { return childCount(); }
	bool setDigitCount(int digitCount)
	{
		if (digitCount == childCount())
			return false;

		while (childCount() != digitCount)
		{
			if (childCount() < digitCount)
				appendChildNode(new DigitNode);
			else
				removeChildNode(lastChild());
		}
		mGeometryDirty = true;
		return true;
	}

	inline int getValue() const { return mValue; }
	inline bool setValue(int value)
	{
		if (value == mValue)
			return false;
		mValue = value;
		mSegmentsDirty = true;
		return true;
	}

	inline int getDigitSize() const { return mDigitSize; }
	inline bool setDigitSize(int digitSize)
	{
		if (digitSize == mDigitSize)
			return false;

		mDigitSize = digitSize;
		// Calculate needed scale to match requested digit size
		mScale = mDigitSize / baseDigitHeight;
		mGeometryDirty = true;
		return true;
	}

	inline SevenSegmentDisplay::Alignment getHAlignment() const { return mHAlignment; }
	inline bool setHAlignment(SevenSegmentDisplay::Alignment hAlignment)
	{
		if (hAlignment == mHAlignment)
			return false;
		mHAlignment = hAlignment;
		mGeometryDirty = true;
		return true;
	}

	inline SevenSegmentDisplay::Alignment getVAlignment() const { return mVAlignment; }
	inline bool setVAlignment(SevenSegmentDisplay::Alignment vAlignment)
	{
		if (vAlignment == mVAlignment)
			return false;
		mVAlignment = vAlignment;
		mGeometryDirty = true;
		return true;
	}

	inline QColor getBgColor() const { return color(); }
	inline bool setBgColor(const QColor& bgColor)
	{
		if (color() == bgColor)
			return false;

		/* Set the background color
		 * This will only mark the material dirty when color differs from current one. */
		setColor(bgColor);
		return true;
	}

	inline const QColor& getOnColor() const { return mOnColor; }
	inline bool setOnColor(const QColor& onColor)
	{
		if (onColor == mOnColor)
			return false;
		mOnColor = onColor;
		mSegmentsDirty = true;
		return true;
	}

	inline const QColor& getOffColor() const { return mOffColor; }
	inline bool setOffColor(const QColor& offColor)
	{
		if (offColor == mOffColor)
			return false;
		mOffColor = offColor;
		mSegmentsDirty = true;
		return true;
	}

	QSizeF update(const QRectF& boundingRectange)
	{
		if (0 == childCount())
			return QSizeF();

		if (mGeometryDirty)
		{
			// Calculate content size
			mContentRect.setWidth(DigitNode::width() * mScale * childCount());
			mContentRect.setHeight(mDigitSize);

			// Update rectangle of the background to the maximum of the size of the given rectangle and the content size
			if (rect().size() != mContentRect.size().expandedTo(boundingRectange.size()))
			{
				setRect(QRectF(QPointF(), mContentRect.size().expandedTo(boundingRectange.size())));
				markDirty(QSGNode::DirtyGeometry);
			}

			// Move the content rectangle to top left of the bounding rectangle
			mContentRect.moveTopLeft(rect().topLeft());

			// Horizontal alignment
			if (mContentRect.width() < rect().width())
			{
				switch (mHAlignment)
				{
				case SevenSegmentDisplay::AlignLeft:
					break;
				case SevenSegmentDisplay::AlignTop:
					qDebug() << "Incompatible alignment: AlignTop as horizontal alignment";
					break;
				case SevenSegmentDisplay::AlignCenter:
					mContentRect.moveCenter(QPointF(rect().center().x(), mContentRect.center().y()));
					break;
				}
			}

			// Verical alignment
			if (mContentRect.height() < rect().height())
			{
				switch (mVAlignment)
				{
				case SevenSegmentDisplay::AlignLeft:
					qDebug() << "Incompatible alignment: AlignLeft as vertical alignment";
					break;
				case SevenSegmentDisplay::AlignTop:
					break;
				case SevenSegmentDisplay::AlignCenter:
					mContentRect.moveCenter(QPointF(mContentRect.center().x(), rect().center().y()));
					break;
				}
			}
		} // geometry update

		// Split the content pane into digit parts
		QRectF digitRect = mContentRect;
		digitRect.setWidth(digitRect.width() / childCount());

		// Update digits
		for (int i = 0; i < childCount(); ++i)
		{
			DigitNode* digit = static_cast<DigitNode*>(childAtIndex(i));

			if (mGeometryDirty)
			{
				// Move digit rect to the right position
				QRectF rect = digitRect;
				rect.moveLeft(rect.left() + rect.width() * i);
				digit->setRect(rect);

				digit->update(mScale);
			}

			if (mSegmentsDirty)
				digit->setValue(mValue, mOnColor, mOffColor);
		}

		mGeometryDirty = false;
		mSegmentsDirty = false;

		return mContentRect.size();
	}

private:
	int mValue = 0;
	int mDigitSize = 24;
	SevenSegmentDisplay::Alignment mHAlignment = SevenSegmentDisplay::AlignLeft;
	SevenSegmentDisplay::Alignment mVAlignment = SevenSegmentDisplay::AlignTop;
	QColor mOnColor = QColor("green");
	QColor mOffColor = QColor("gray");

	bool mGeometryDirty = true;
	bool mSegmentsDirty = true;
	qreal mScale = mDigitSize / baseDigitHeight;
	QRectF mContentRect;
};

#endif /* DISPLAYNODE_P_HPP_ */
