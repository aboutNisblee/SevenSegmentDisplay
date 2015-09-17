/**
 * \file displaynode_p.hpp
 *
 * \date 14.09.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef DISPLAYNODE_P_HPP_
#define DISPLAYNODE_P_HPP_

#include "sevensegmentdisplay.hpp"

#include <memory>
#include <algorithm>
#include <array>
#include <cmath>

#include <QSGGeometryNode>
#include <QSGSimpleRectNode>
#include <QMatrix>

namespace
{
/* Sizes in un-scaled coordinate system. */
Q_CONSTEXPR qreal baseSegLength = 2.0;
Q_CONSTEXPR qreal baseSegWidth = 0.60;
Q_CONSTEXPR qreal baseSegGap = 0.45 / 10;
Q_CONSTEXPR qreal baseDigitHeight = 2 * baseSegLength + baseSegWidth + 4 * baseSegGap;

Q_CONSTEXPR qreal baseDotRadius = baseSegWidth * 0.6;
Q_CONSTEXPR quint8 dotSegs = 24;

/* 0 0×3F, 1 0×06, 2 0×5B, 3 0×4F, 4 0×66, 5 0×6D, 6 0×7D, 7 0×07, 8 0×7F, 9 0×6F */
Q_CONSTEXPR quint8 lutSegCode[] =
{ 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f };
} // namespace

/** \internal Common base class for scene graph nodes. */
template<int count>
class ElementNode: public QSGGeometryNode
{
public:
	virtual ~ElementNode() {}
	/** \internal Update the color of the element. */
	inline void setColor(const QColor& color)
	{
		if (mMaterial->color() == color)
			return;

		mMaterial->setColor(color);
		markDirty(QSGNode::DirtyMaterial);
	}
	/** \internal Update the geometry by mapping all vertices into the coordinate system of the given matrix. */
	inline void updateGeometry(const QMatrix& mat)
	{
		bool dirty = false;
		QSGGeometry::Point2D* data = mGeometry->vertexDataAsPoint2D();
		for (int i = 0; i < mGeometry->vertexCount(); ++i)
		{
			// QPointF is is actually QPointDouble! So take care in comparisons!
			QPointF p = mat.map(mVertices[i]);
			float pX = static_cast<float>(p.x());
			float pY = static_cast<float>(p.y());
			if (!qFuzzyCompare(pX, data[i].x) || !qFuzzyCompare(pY, data[i].y))
			{
				data[i].set(pX, pY);
				dirty = true;
			}
		}

		if (dirty)
			markDirty(QSGNode::DirtyGeometry);
	}
	/** \internal Returns the effective vertex position (only for debugging). */
	inline QPointF getEffectiveVertex(qint8 no)
	{
		if (no < mGeometry->vertexCount())
			return QPointF(mGeometry->vertexDataAsPoint2D()[no].x, mGeometry->vertexDataAsPoint2D()[no].y);
		return QPointF();
	}
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
};

/** \internal Scene graph geometry node of a dot. */
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
};

/** \internal Scene graph node for a single digit (7 segments + dot). */
struct DigitNode: public QSGNode
{
	/** \internal Construct a new digit. */
	DigitNode()
	{
		// Lifetime is managed by scene graph
		appendChildNode(new SegmentNode());
		appendChildNode(new SegmentNode(90));
		appendChildNode(new SegmentNode(90));
		appendChildNode(new SegmentNode());
		appendChildNode(new SegmentNode(90));
		appendChildNode(new SegmentNode(90));
		appendChildNode(new SegmentNode());
		appendChildNode(new DotNode);
	}

	/** \internal Update the geometry of this digit.
	 * \param rectangle A rectangle in which to layout the digit.
	 * \param scale The factor to adjust the basic segment sizes.
	 */
	inline void updateGeometry(QRectF rectangle, qreal scale)
	{
		qreal segWidth = baseSegWidth * scale;
		qreal segLength = baseSegLength * scale;
		qreal segGap = baseSegGap * scale;
		qreal dotRadius = baseDotRadius * scale;

		/* Segments are positioned around the center of the digit, excluding the dot.
		 * So the digit center must be left justified by the half of the space the dot needs. */
		qreal digitCenterX = rectangle.center().x() - (2 * dotRadius + segGap) / 2;
		qreal digitCenterY = rectangle.center().y();

		Q_ASSERT(8 == childCount());

		// QMatrix(qreal m11, qreal m12, qreal m21, qreal m22, qreal dx, qreal dy)
		// ->       hScaling, vShearing, hShearing,  vScaling,   hTrans,   vTrans

		// (A) top
		static_cast<SegmentNode*>(childAtIndex(0))->updateGeometry(QMatrix(scale, 0, 0, scale, digitCenterX,
		        digitCenterY - segLength - segGap * 2));
		// (B) top right
		static_cast<SegmentNode*>(childAtIndex(1))->updateGeometry(QMatrix(scale, 0, 0, scale,
		        digitCenterX + segLength / 2 + segGap, digitCenterY - segLength / 2 - segGap));
		// (C) bottom right
		static_cast<SegmentNode*>(childAtIndex(2))->updateGeometry(QMatrix(scale, 0, 0, scale,
		        digitCenterX + segLength / 2 + segGap, digitCenterY + segLength / 2 + segGap));
		// (D) bottom
		static_cast<SegmentNode*>(childAtIndex(3))->updateGeometry(QMatrix(scale, 0, 0, scale, digitCenterX,
		        digitCenterY + segLength + segGap * 2));
		// (E) bottom left
		static_cast<SegmentNode*>(childAtIndex(4))->updateGeometry(QMatrix(scale, 0, 0, scale,
		        digitCenterX - segLength / 2 - segGap, digitCenterY + segLength / 2 + segGap));
		// (F) top left
		static_cast<SegmentNode*>(childAtIndex(5))->updateGeometry(QMatrix(scale, 0, 0, scale,
		        digitCenterX - segLength / 2 - segGap, digitCenterY - segLength / 2 - segGap));
		// (G) middle
		static_cast<SegmentNode*>(childAtIndex(6))->updateGeometry(QMatrix(scale, 0, 0, scale, digitCenterX, digitCenterY));

		// The dot is always laid out. Appearance is controlled by color.
		static_cast<DotNode*>(childAtIndex(7))->updateGeometry(QMatrix(scale, 0, 0, scale,
		        digitCenterX + segLength / 2 + segGap + segWidth / 2 + dotRadius +
		        segGap, digitCenterY + segLength + 2 * segGap + segWidth / 2 - dotRadius));

#if 0
		qDebug() << "Effective digit size:" << static_cast<SegmentNode*>(childAtIndex(3))->getEffectiveVertex(
		             1).y() - static_cast<SegmentNode*>(childAtIndex(0))->getEffectiveVertex(
		             2).y();
#endif
	}

	inline void setValue(quint8 value, const QColor& onColor, const QColor& offColor)
	{
		Q_ASSERT(8 == childCount());

		if (value < 10)
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

		// Disable dot
		static_cast<DotNode*>(lastChild())->setColor(offColor);
	}

	inline void setMinus(const QColor& onColor, const QColor& offColor)
	{
		for (quint8 i = 0; i < 7; ++i)
		{
			/* Segment material is only marked dirty, when color is changed. */
			static_cast<SegmentNode*>(childAtIndex(i))->setColor((6 == i) ? onColor : offColor);
		}
		// Disable dot
		static_cast<DotNode*>(lastChild())->setColor(offColor);
	}

	inline void setDot(const QColor& onColor)
	{
		static_cast<DotNode*>(lastChild())->setColor(onColor);
	}

	static Q_DECL_CONSTEXPR qreal width()
	{
		return baseSegLength  + baseSegWidth  + 2 * baseSegGap  + 2 * baseDotRadius  + baseSegGap ;
	}
};

/** \internal Root scene graph node of the display. */
class DisplayNode: public QObject, public QSGSimpleRectNode
{
	Q_OBJECT

public:
	inline int getDigitCount() const { return mDigitCount; }
	bool setDigitCount(int digitCount)
	{
		if (digitCount == mDigitCount)
			return false;

		mDigitCount = digitCount;
		return true;
	}

	inline double getValue() const { return mValue; }
	inline bool setValue(double value)
	{
		if (value == mValue)
			return false;
		mValue = value;
		mSegmentsDirty = true;
		return true;
	}

	inline int getPrecision() const { return mValue; }
	inline bool setPrecision(int precision)
	{
		if (precision == mPrecision)
			return false;
		mPrecision = precision;
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

	inline QColor getBgColor() const { return mBgColor; }
	inline bool setBgColor(const QColor& bgColor)
	{
		if (bgColor == mBgColor)
			return false;

		mBgColor = bgColor;
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

	/** \internal Update the display.
	 * This method should be called from render thread.
	 * @param boundingRectange The bounding rectangle of the widget.
	 * @return The content size. Can be used to set the implicitSize of the widget.
	 */
	QSizeF update(const QRectF& boundingRectange)
	{
		// Check digit count
		while (childCount() != mDigitCount)
		{
			if (childCount() < mDigitCount)
				appendChildNode(new DigitNode);
			else
				removeChildNode(lastChild());

			mGeometryDirty = true;
			mSegmentsDirty = true;
		}

		if (rect() != boundingRectange)
			mGeometryDirty = true;

		if (mGeometryDirty)
		{

			// Calculate content size
			mContentRect.setHeight(mDigitSize);
			mContentRect.setWidth(DigitNode::width() * mScale * childCount());

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

		// Update geometry of digits
		if (mGeometryDirty)
		{
			// Split the content area into digit parts
			QRectF digitRect = mContentRect;
			digitRect.setWidth(digitRect.width() / childCount());

			for (int i = 0; i < childCount(); ++i)
			{
				DigitNode* digit = static_cast<DigitNode*>(childAtIndex(i));

				// Move digit rectangle to the right position
				QRectF rect = digitRect;
				rect.moveLeft(rect.left() + rect.width() * i);
				digit->updateGeometry(rect, mScale);
			}
		}

		/* TODO: This is crap! No negative values, overflow problems and no literals
		 * Maybe better work on strings?! Have a look at the implementation of QLCDNumber. */
		if (mSegmentsDirty)
		{
			double integral = 0;
			double fraction = std::modf(mValue, &integral);

			int pos = setInteger(0, childCount() - mPrecision, static_cast<qint64>(integral));
			if (pos >= 0)
			{
				if (mPrecision > childCount())
					qWarning() << "Invalid precision:" << mPrecision << "( digitCount:" << childCount() << ")";
				else if (mPrecision > 0 && mPrecision <= childCount())
				{
					// Set dot
					if (pos > 0)
					{
						DigitNode* digit = static_cast<DigitNode*>(childAtIndex(pos - 1));
						digit->setDot(mOnColor);
					}

					qint64 fracAsInt = std::round(fraction * std::pow(10, mPrecision));
					setInteger(pos, childCount(), fracAsInt);
				}
			}
		}

		/* Set the background color
		 * This will only mark the material dirty when color differs from current one. */
		setColor(mBgColor);

		mGeometryDirty = false;
		mSegmentsDirty = false;

		return mContentRect.size();
	}

signals:
	void overflow();

private:
	int setInteger(int first, int last, qint64 value)
	{
		if (first < last)
		{
			int neededPositions = 1;
			if (value != 0)
				neededPositions = static_cast<int>(std::log10(std::abs(value))) + ((value < 0) ? 2 : 1);

			if (neededPositions > last - first)
			{
				emit overflow();
				//				value = value % static_cast<qint64>(std::pow(10, last - first));
				for (int i = 0; i < childCount(); ++i)
					static_cast<DigitNode*>(childAtIndex(i))->setMinus(mOnColor, mOffColor);
				return -1;
			}

			if (value < 0)
			{
				DigitNode* digit = static_cast<DigitNode*>(childAtIndex(first));
				digit->setMinus(mOnColor, mOffColor);
				++first;

				value = std::abs(value);
			}

			//			qDebug() << "Pos:" << first << ":" << static_cast<int>(value / std::pow(10, last - first - 1));

			DigitNode* digit = static_cast<DigitNode*>(childAtIndex(first));
			digit->setValue(value / std::pow(10, last - first - 1), mOnColor, mOffColor);
			return setInteger(first + 1, last, static_cast<qint64>(std::fmod(value, std::pow(10, last - first - 1))));
		}
		else
			return first;
	}

	double mValue;
	int mDigitCount = 1;
	int mPrecision = 0;
	int mDigitSize = 24;
	SevenSegmentDisplay::Alignment mHAlignment = SevenSegmentDisplay::AlignLeft;
	SevenSegmentDisplay::Alignment mVAlignment = SevenSegmentDisplay::AlignTop;
	QColor mBgColor = QColor(Qt::transparent);
	QColor mOnColor = QColor("green");
	QColor mOffColor = QColor("gray");

	bool mGeometryDirty = true;
	bool mSegmentsDirty = true;
	qreal mScale = mDigitSize / baseDigitHeight;
	QRectF mContentRect;
};

#endif /* DISPLAYNODE_P_HPP_ */
