/**
 * \file sevensegmentdisplay.cpp
 *
 * \date 04.09.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include <gui/sevensegmentdisplay.hpp>
#include <gui/displaynode_p.hpp>

class SevenSegmentDisplayPrivate
{
public:
	SevenSegmentDisplayPrivate():
		mDisplayNode(new DisplayNode)
	{
	}
	Q_DISABLE_COPY(SevenSegmentDisplayPrivate)

	bool display(QVariant v)
	{
		bool updateNeeded = false;

		mCurrentValue = v;

		// Update value
		switch (v.type())
		{
		case QVariant::Double:
		{
			QString s;

			/* NOTE: In no case does a nonexistent or small field width cause truncation of a field; if the result of a con‐
			 * version is wider than the field width, the field is expanded to contain the conversion result. */
			if (mPrecision > 0)
				s.sprintf("%*.*f", mDisplayNode->getDigitCount() + 1 - mPrecision, mPrecision, v.toDouble());
			else
				s.sprintf("%*.0f", mDisplayNode->getDigitCount(), v.toDouble());

			updateNeeded =  mDisplayNode->setString(s);
		}
		break;
		case QVariant::String:
			updateNeeded =  mDisplayNode->setString(v.toString());
			break;
		default:
			qWarning() << "BUG: Unhandled type in mCurrentValue: (" << v.typeName() << ")";
			break;
		}

		return updateNeeded;
	}

	DisplayNode* mDisplayNode; // Owned by scene graph
	QVariant mCurrentValue;
	int mPrecision = 0;
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
	if (count < 0)
		qWarning() << "Digit count cannot be negative";
	else if (d->mDisplayNode->setDigitCount(count))
	{
		update();
		emit digitCountChanged();

		// Update value
		d->display(d->mCurrentValue);
	}
}

double SevenSegmentDisplay::getValue() const
{
	Q_D(const SevenSegmentDisplay);
	bool ok = false;
	double result = d->mDisplayNode->getString().toDouble(&ok);
	if (ok)
		return result;
	else
		return 0;
}
void SevenSegmentDisplay::setValue(double value)
{
	Q_D(SevenSegmentDisplay);
	if (d->display(QVariant(value)))
	{
		update();
		emit valueChanged();
	}
}

QString SevenSegmentDisplay::getString() const { Q_D(const SevenSegmentDisplay); return d->mDisplayNode->getString(); }
void SevenSegmentDisplay::setString(QString string)
{
	Q_D(SevenSegmentDisplay);

	// TODO: Check if string is printable or is it the responsibility of the node?
	if (d->display(QVariant(string)))
	{
		update();
		emit stringChanged();
	}
}

int SevenSegmentDisplay::getPrecision() const { Q_D(const SevenSegmentDisplay); return d->mPrecision; }
void SevenSegmentDisplay::setPrecision(int precision)
{
	Q_D(SevenSegmentDisplay);
	if (precision < 0)
		qWarning() << "Precision cannot be negative";
	else if (d->mPrecision != precision)
	{
		d->mPrecision = precision;

		emit precisionChanged();

		// Update value
		if (d->display(d->mCurrentValue))
			update();
	}
}

int SevenSegmentDisplay::getDigitSize() const { Q_D(const SevenSegmentDisplay); return d->mDisplayNode->getDigitSize(); }
void SevenSegmentDisplay::setDigitSize(int size)
{
	Q_D(SevenSegmentDisplay);
	if (d->mDisplayNode->setDigitSize(size))
	{
		update();
		emit digitSizeChanged();
	}
}

SevenSegmentDisplay::Alignment SevenSegmentDisplay::getVerticalAlignment() const { Q_D(const SevenSegmentDisplay); return d->mDisplayNode->getVAlignment(); }
void SevenSegmentDisplay::setVerticalAlignment(Alignment alignment)
{
	Q_D(SevenSegmentDisplay);
	if (d->mDisplayNode->setVAlignment(alignment))
	{
		update();
		emit verticalAlignmentChanged();
	}
}

SevenSegmentDisplay::Alignment SevenSegmentDisplay::getHorizontalAlignment() const { Q_D(const SevenSegmentDisplay); return d->mDisplayNode->getHAlignment(); }
void SevenSegmentDisplay::setHorizontalAlignment(Alignment alignment)
{
	Q_D(SevenSegmentDisplay);
	if (d->mDisplayNode->setHAlignment(alignment))
	{
		update();
		emit horizontalAlignmentChanged();
	}
}

QColor SevenSegmentDisplay::getBgColor() const { Q_D(const SevenSegmentDisplay); return d->mDisplayNode->getBgColor(); }
void SevenSegmentDisplay::setBgColor(const QColor& color)
{
	Q_D(SevenSegmentDisplay);
	if (d->mDisplayNode->setBgColor(color))
	{
		update();
		emit bgColorChanged();
	}
}

QColor SevenSegmentDisplay::getOnColor() const { Q_D(const SevenSegmentDisplay); return d->mDisplayNode->getOnColor(); }
void SevenSegmentDisplay::setOnColor(const QColor& color)
{
	Q_D(SevenSegmentDisplay);
	if (d->mDisplayNode->setOnColor(color))
	{
		update();
		emit onColorChanged();
	}
}

QColor SevenSegmentDisplay::getOffColor() const { Q_D(const SevenSegmentDisplay); return d->mDisplayNode->getOffColor(); }
void SevenSegmentDisplay::setOffColor(const QColor& color)
{
	Q_D(SevenSegmentDisplay);
	if (d->mDisplayNode->setOffColor(color))
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
