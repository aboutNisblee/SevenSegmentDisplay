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
	QColor mBgColor = QColor("black");
	QColor mOnColor = QColor("green");
	QColor mOffColor = QColor("gray");
};

namespace
{

// XXX: Configurable??
/* Length and width of a segment in un-scaled coordinate system. */
constexpr qreal baseLength = 2.0;
constexpr qreal baseWidth = 0.45;

/** \internal Scene graph geometry node of a single segment. */
struct SegmentNode: public QSGGeometryNode
{
	SegmentNode()
	{
		mGeo = std::unique_ptr<QSGGeometry>(new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), mV.size()));
		mGeo->setDrawingMode(GL_TRIANGLE_STRIP);
		setGeometry(mGeo.get());

		mMat = std::unique_ptr<QSGFlatColorMaterial>(new QSGFlatColorMaterial);
		mMat->setColor(QColor("green"));
		setMaterial(mMat.get());

		reset();
	}

	/** \internal Reset the segment to its initial state. */
	inline SegmentNode& reset()
	{
		/* Vertices are placed in the center of a 2-dimensional coordinate system to simplify initial rotation.
		 * The y values are increasing downwards to ease mapping to Quicks coordinate system. */
		mV[0] = { -baseLength / 2, 0};
		mV[1] = { -baseLength / 2 + baseWidth / 2, baseWidth / 2};
		mV[2] = { -baseLength / 2 + baseWidth / 2, -baseWidth / 2};
		mV[3] = {baseLength / 2 - baseWidth / 2, baseWidth / 2};
		mV[4] = {baseLength / 2 - baseWidth / 2, -baseWidth / 2};
		mV[5] = {baseLength / 2, 0};
		return *this;
	}

	/** \internal Rotate the segment. */
	inline SegmentNode& rotate(qreal deg)
	{
		QMatrix m = QMatrix().rotate(deg);
		for (QPointF& v : mV)
			v = m.map(v);
		return *this;
	}

	/** \internal Update the geometry by mapping the current segment into the coordinate system of the given matrix. */
	inline void updateGeometry(const QMatrix& trans)
	{
		QSGGeometry::Point2D* data = mGeo->vertexDataAsPoint2D();
		for (int i = 0; i < mGeo->vertexCount(); ++i)
		{
			QPointF p = trans.map(mV[i]);
			data[i].set(p.x(), p.y());
		}
		markDirty(QSGNode::DirtyGeometry);
	}

	/** \internal Update the forground color of the segment. */
	inline void setColor(const QColor& color)
	{
		if (mMat->color() != color)
		{
			mMat->setColor(color);
			markDirty(QSGNode::DirtyMaterial);
		}
	}

	std::array<QPointF, 6> mV;
	std::unique_ptr<QSGGeometry> mGeo;
	std::unique_ptr<QSGFlatColorMaterial> mMat;
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
	}

	/** \internal Update the state of the node and all its children.
	 * Things are only updated when the settings differ from last call.
	 * @param rectangle The current binding rectangle of the caller.
	 */
	void update(const QRectF& rectangle)
	{
		// Geometry
		// XXX: Remember to also check for segment geometry changes, when made it configurable!
		if (rect() != rectangle)
		{
			setRect(rectangle);
			markDirty(QSGNode::DirtyGeometry);

			QPointF center(rectangle.width() / 2 * 0.8, rectangle.height() / 2);
			qreal width = rectangle.width() * 0.4;
			qreal scale = width / baseLength;
			// Set scale for all segments
			QMatrix matScale = QMatrix().scale(scale, scale);
			qreal gap = baseWidth / 10 * scale;

			mSegments[0]->updateGeometry(matScale * QMatrix().translate(center.x(), center.y() - baseLength * scale - gap * 2));
			mSegments[1]->updateGeometry(matScale * QMatrix().translate(center.x() + width / 2 + gap,
			                             center.y() - baseLength / 2 * scale - gap));
			mSegments[2]->updateGeometry(matScale * QMatrix().translate(center.x() + width / 2 + gap,
			                             center.y() + baseLength / 2 * scale + gap));
			mSegments[3]->updateGeometry(matScale * QMatrix().translate(center.x(), center.y() + baseLength * scale + gap * 2));
			mSegments[4]->updateGeometry(matScale * QMatrix().translate(center.x() - width / 2 - gap,
			                             center.y() + baseLength / 2 * scale + gap));
			mSegments[5]->updateGeometry(matScale * QMatrix().translate(center.x() - width / 2 - gap,
			                             center.y() - baseLength / 2 * scale - gap));
			mSegments[6]->updateGeometry(matScale * QMatrix().translate(center.x(), center.y()));
		}

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
	}

private:
	/* TODO: Add dot! */
	std::array<std::unique_ptr<SegmentNode>, 7> mSegments;
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

	connect(this, &QQuickItem::widthChanged, this, [&]()
	{
		if (height() / width() != 4 / 3)
			setHeight(width() / 3 * 4);
	});
	connect(this, &QQuickItem::heightChanged, this, [&]()
	{
		if (height() / width() != 4 / 3)
			setWidth(height() / 4 * 3);
	});
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
	digit->update(boundingRect());
	return digit;
}
